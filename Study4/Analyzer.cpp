#include "Keyboard.h"
#include "DataReader.h"

#include <map>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

const int USER_L = 1;
const int ALG_NUM = 1;
const int RANK = 13;

const int SAMPLE_NUM = 32;

double dtw[MAXSAMPLE][MAXSAMPLE];

int block, wordCount[2]; //[type]
double wpm[2];
char qwertyRating[USER_NUM], gestureKeyboardRating[USER_NUM];

int rk[ALG_NUM];
double result[ALG_NUM];
double rkCount[2][ALG_NUM][RANK + 1]; //[type][std;dtw;dtw*freq][rank]

string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];

map<string, int> dict_map;
map<string, double> katz_alpha;
map<string, ll> unigram_map;
map<pair<string, string>, double> bigram_map;
vector<Vector2> dict_location[LEXICON_SIZE][2], dict_shape[LEXICON_SIZE][2]; //[scale: 0 for 1x1, 1 for 1x3]

fstream keyFout, WPMFout, timeFout, disFout, speedFout;

void initLexicon()
{
    fstream fin;
    fin.open("ANC-written-noduplicate+pangram.txt", fstream::in);
    string s;
    rep(i, LEXICON_SIZE)
    {
        fin >> dict[i] >> freq[i];
        dict_map[dict[i]] = freq[i];
        dict_location[i][0] = temporalSampling(wordToPath(dict[i], 1), SAMPLE_NUM);
        dict_location[i][1] = temporalSampling(wordToPath(dict[i], 3), SAMPLE_NUM);
        dict_shape[i][0] = normalize(dict_location[i][0]);
        dict_shape[i][1] = normalize(dict_location[i][1]);
    }
    fin.close();
    fin.open("bigrams-LDC-10k-katz-1m-phrase.txt", fstream::in);
    string s1, s2;
    int bigrams_num;
    ll freq;
    double prob;

    fin >> bigrams_num;
    cout << "Bigram pairs: " << bigrams_num << endl;
    rep(i, bigrams_num)
    {
        fin >> s1 >> s2 >> prob;
        bigram_map[mk(s1, s2)] = prob;
    }
    int unigram_size;
    fin >> unigram_size;
    rep(i, unigram_size)
    {
        fin >> s1 >> freq >> prob;
        unigram_map[s1] = freq;
        katz_alpha[s1] = prob;
    }
    fin.close();
}

void readQuestionnaire()
{
    fstream fin;
    fin.open("questionnaire/Questionnaire_Study2.csv", fstream::in);
    string s;
    getline(fin, s);
    rep(id, USER_NUM)
    {
        getline(fin, s);
        int cnt = 0;
        rep(i, s.length())
            if (s[i] == ',')
            {
                cnt++;
                if (cnt == 4)
                {
                    qwertyRating[id] = s[i + 1];
                    gestureKeyboardRating[id] = s[i + 3];
                    break;
                }
            }
    }
    fin.close();
}

void init()
{
    WPMFout.open("res/WPM_Study2.csv", fstream::out);
    WPMFout << "id,QWERTY,GK,usage,block,WPM,words,top1,top2,top3,top4,top5,top6,top7,top8,top9,top10,top11,top12,top13" << endl;
    timeFout.open("res/Time_Study2.csv", fstream::out);
    timeFout << "id,QWERTY,GK,usage,block,sentence,Gesture,Prepare,Gesture(%),Prepare(%),Gesture(M),Prepare(M),GestureSpeed(keyW/s)" << endl;
    speedFout.open("res/Speed_Study2.csv", fstream::out);
    speedFout << "id,QWERTY,GK,usage,block,sentence,word,Time(#/100),GestureSpeed(keyW/s)" << endl;
    initKeyboard(dtw);
    initLexicon();
    readQuestionnaire();
}

bool outKeyboard(Vector2 v, float sc)
{
    return v.x > 0.5 || v.x < -0.5 || v.y > (0.5 * 0.3 * sc) || v.y < -(0.5 * 0.3 * sc);
}

double calcBigramProb(int w, int now_id = -1)
{
    double bi_f;
    string pre = (w)?words[w-1]:"<s>";
    string now = (now_id == -1)?words[w]:dict[now_id];
    bi_f = bigram_map[mk(pre, now)];
    if (bi_f == 0)
    {
        bi_f = katz_alpha[pre] * unigram_map[now];
        if (bi_f == 0)
            return -INF;
    }
    return log(bi_f);
}


void calcCandidate(int id)
{
    int line = 0, p = 0, sc = 3;

    if (mode[id] == "Direct")
        p = 1;

    rep(w, words.size())
    {
        string word = words[w];
        vector<Vector2> rawstroke;
        while (line < cmd.size())
        {
            string s = cmd[line];
            Vector2 p(relative[line].x, relative[line].y * 0.3 * sc);
            line++;
            if (rawstroke.size() == 0 || dist(rawstroke[rawstroke.size()-1], p) > eps)
                rawstroke.push_back(p);
            if (s == "Ended")
                break;
        }
        wordCount[p]++;
        int num = SAMPLE_NUM;
        vector<Vector2> location = temporalSampling(wordToPath(word, sc), num);
        vector<Vector2> stroke = temporalSampling(rawstroke, num);

        double f = dict_map[word], bi_f = calcBigramProb(w);
        if (f == 0)
        {
            f = freq[LEXICON_SIZE - 1];
            cout << "Warning: OOV!" << endl;
        }
        result[0] = bi_f - 80 * match(stroke, location, dtw, DTW);
        rk[0] = 1;
        rep(j, LEXICON_SIZE)
        {
            if (word == dict[j])
                continue;
            vector<Vector2>& location = dict_location[j][1];
            if (dist(location.back(), stroke.back()) > 0.3)
                continue;
            double disDTW = match(stroke, location, dtw, DTW);
            bi_f = calcBigramProb(w, j);
            if (bi_f - 80 * disDTW > result[0])
                rk[0]++;
        }
        if (rk[0] <= RANK)
            rkCount[p][0][rk[0]]++;
    }
    int alpha = sentence[id].length();
    double startT = time[0], lastT = time[time.size() - 2];
    wpm[p] += alpha / (lastT - startT) * 12;
}

void outputBasicInfo(fstream& fout, int userID, int id)
{
    string usage = (mode[id]=="Direct")?"Direct":"Indirect";
    fout << userID + 1 << ","
         << qwertyRating[userID] << ","
         << gestureKeyboardRating[userID] << ","
         << usage << ","
         << block << ",";
}

void outputBlock(int userID, int id, int p)
{
    fstream& fout = WPMFout;
    For(j, RANK)
        rkCount[p][0][j] += rkCount[p][0][j-1];
    outputBasicInfo(fout, userID, id);

    fout << wpm[p] / 10 << "," << wordCount[p];
    For(j, RANK)
        fout << "," << rkCount[p][0][j] / wordCount[p];
    fout << endl;
}

void outputTime(int userID, int id)
{
    double gesture = 0, prepare = 0, speed = 0, len = 0, draw = 0, keyW = width[id] / 10;
    int wordCount = 0, curWord = 0, gestureCnt = 0, prepareCnt = 0, l = 0, r = 0;
    while (l < cmd.size() - 1)
    {
        r = l;
        while (cmd[r] != "Ended")
        {
            len += dist(world[r], world[r+1]);
            r++;
        }
        draw = time[r] - time[l];
        gesture += draw;
        gestureCnt ++;
        if (len > eps)
        {
            speed += len / keyW /  draw;
            wordCount++;

            double t_block = draw / 100;
            int part = 1;
            double d = 0, spend = 0, p = 0;
            FOR(i, l, r - 1)
            {
                double ddist = dist(world[i+1], world[i]), dtime = time[i+1] - time[i];
                while (spend + dtime > t_block)
                {
                    p = (t_block - spend) / dtime;
                    d += ddist * p;
                    outputBasicInfo(speedFout, userID, id);
                    speedFout << ((id>=40)?id-39:id+1) << ","
                              << words[curWord] << ","
                              << part++ << ","
                              << d / keyW / t_block << endl;
                    ddist *= 1 - p;
                    dtime -= t_block - spend;
                    d = spend = 0;
                }
                d += ddist;
                spend += dtime;
            }
            if (part <= 100)
            {
                outputBasicInfo(speedFout, userID, id);
                speedFout << ((id>=40)?id-39:id+1) << ","
                          << words[curWord] << ","
                          << part << ","
                          << d / keyW / t_block << endl;
            }

        }

        l = r + 1;
        len = draw = 0;
        curWord++;
        if (cmd[l] == "Began")
        {
            prepare += time[l] - time[l - 1];
            prepareCnt ++;
        }

    }

    if (curWord != words.size())
    {
        cout << words.size() << " " << curWord << endl;
        cout << "Word Count Error" << endl;
        while(1);
    }

    double tot = gesture + prepare;
    outputBasicInfo(timeFout, userID, id);
    timeFout << ((id>=40)?id-39:id+1) << ","
             << gesture << ","
             << prepare << ","
             << gesture / tot << ","
             << prepare / tot << ","
             << gesture / gestureCnt << ","
             << prepare / prepareCnt << ","
             << speed / wordCount << endl;
}

void clean()
{
    memset(wpm, 0, sizeof(wpm));
    memset(rkCount, 0, sizeof(rkCount));
    memset(wordCount, 0, sizeof(wordCount));
}

stringstream ss;

int main()
{
    init();
    FOR(p, USER_L - 1, USER_NUM - 1)
    {
        block = 1;
        rep(i, 40)
        {
            ss.clear();ss.str("");
            ss << i;
            string fileName = "data/" + user[p] + "_" + ss.str() + ".txt";
            readData(fileName, i);
            calcCandidate(i);
            outputTime(p, i);
            if ((i+1) % 10 == 0)
            {
                outputBlock(p, i, 0);
                block++;
                clean();
            }
        }
        block = 1;
        FOR(i, 40, PHRASES - 1)
        {
            ss.clear();ss.str("");
            ss << i - 39;
            string fileName = "data/Direct/" + user[p] + "_" + ss.str() + ".txt";
            readData(fileName, i);
            calcCandidate(i);
            outputTime(p, i);
            if ((i+1) % 10 == 0)
            {
                outputBlock(p, i, 1);
                block++;
                clean();
            }
        }
    }
    WPMFout.close();
    timeFout.close();
    speedFout.close();
    return 0;
}

