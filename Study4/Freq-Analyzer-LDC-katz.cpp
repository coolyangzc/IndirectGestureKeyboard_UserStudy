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
const int RANK = 13;

const int GAMMA_NUM = 300;
const double DELTA_G = 1;
const int SAMPLE_NUM = 32;


int rk[GAMMA_NUM];
double result[GAMMA_NUM];
double rkCount[2][GAMMA_NUM][RANK + 1]; //[type][][rank]

double dtw[MAXSAMPLE][MAXSAMPLE];

int wordCount[2]; //[type]

string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];
map<string, int> dict_map;
map<string, double> katz_alpha;
map<string, ll> unigram_map;
map<pair<string, string>, double> bigram_map;
vector<Vector2> dict_location[LEXICON_SIZE][2], dict_shape[LEXICON_SIZE][2]; //[scale: 0 for 1x1, 1 for 1x3]

string userID;
fstream bigramFout;

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

void init()
{
    string freqFileName = "res/Bigram_Freq_LDC_Katz_Study2.csv";
    bigramFout.open(freqFileName.c_str(), fstream::out);
    bigramFout << "id,usage,gamma,top1,top2,top3,top4,top5,top6,top7,top8,top9,top10,top11,top12,top13" << endl;
    initKeyboard(dtw);
    initLexicon();
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
    fstream& fout = bigramFout;
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
        result[0] = match(stroke, location, dtw, DTW);

        rep(i, GAMMA_NUM)
            rk[i] = 1;
        FOR(i, 6, GAMMA_NUM - 1)
            result[i] = bi_f - (i * DELTA_G) * result[0];
        rep(j, LEXICON_SIZE)
        {
            if (word == dict[j])
                continue;
            vector<Vector2>& location = dict_location[j][1];
            if (dist(location.back(), stroke.back()) > 0.3)
                continue;
            //double disSHARK = match(stroke, location, dtw, Standard);
            double disDTW = match(stroke, location, dtw, DTW);
            bi_f = calcBigramProb(w, j);
            FOR(i, 6, GAMMA_NUM - 1)
                if (rk[i] <= RANK && bi_f - (i * DELTA_G) * disDTW > result[i])
                    rk[i] ++;
        }
        rep(i, GAMMA_NUM)
            if (rk[i] <= RANK)
                rkCount[p][i][rk[i]]++;
    }
}

void outputCandidate()
{
    fstream& fout = bigramFout;
    rep(p, 2)
    {
        string usage = (p==0)?"Indirect":"Direct";
        FOR(i, 6, GAMMA_NUM - 1)
        {
            For(j, RANK)
                rkCount[p][i][j] += rkCount[p][i][j-1];
            fout<< userID << ","
                << usage << ","
                << i * DELTA_G;
            For(j, RANK)
                fout << "," << rkCount[p][i][j] / wordCount[p];
            fout << endl;
        }
    }
}

void clean()
{
    memset(rkCount, 0, sizeof(rkCount));
    memset(wordCount, 0, sizeof(wordCount));
}

stringstream ss;

int main()
{
    init();
    FOR(p, USER_L - 1, USER_NUM - 1)
    {
        ss.clear();ss.str("");
        ss << p + 1;
        userID = ss.str();
        rep(i, 40)
        {
            ss.clear();ss.str("");
            ss << i;
            string fileName = "data/" + user[p] + "_" + ss.str() + ".txt";
            readData(fileName, i);
            calcCandidate(i);
        }

        FOR(i, 40, PHRASES - 1)
        {
            ss.clear();ss.str("");
            ss << i - 39;
            string fileName = "data/Direct/" + user[p] + "_" + ss.str() + ".txt";
            readData(fileName, i);
            calcCandidate(i);
        }
        outputCandidate();
        clean();
    }
    bigramFout.close();
    return 0;
}

