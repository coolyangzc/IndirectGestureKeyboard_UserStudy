#include "Common.h"
#include "Vector2.h"
#include "Keyboard.h"

#include <map>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

const int USER_L = 18;

const int THETA_NUM = 100;
const double DELTA_T = 0.0005;

int rk[THETA_NUM];
double result[THETA_NUM];
double rkCount[2][3][THETA_NUM][13]; //[scale][size][][rank]

const int SAMPLE_NUM = 32;

double dtw[MAXSAMPLE][MAXSAMPLE];

string sentence[PHRASES], mode[PHRASES], scale[PHRASES];
double height[PHRASES], width[PHRASES], heightRatio[PHRASES], widthRatio[PHRASES], keyboardSize[PHRASES];

int wordCount[2][3]; //[scale][size]

string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];
map<string, int> dict_map;
vector<Vector2> dict_location[LEXICON_SIZE][2]; //[scale: 0 for 1x1, 1 for 1x3]

vector<string> cmd, words;

vector<double> time;
vector<Vector2> world, relative;

string name, userID;
fstream freqFout;

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
    }
    fin.close();
}

void init()
{
    string candFileName = "res/Freq_Study1.csv";
    freqFout.open(candFileName.c_str(), fstream::out);
    freqFout << "id,scale,size,theta(keywidth),top1,top2,top3,top4,top5,top6,top7,top8,top9,top10,top11,top12" << endl;
    initKeyboard(dtw);
    initLexicon();
}

void linePushBack(string s, double t, double x = 0, double y = 0, double rx = 0, double ry = 0)
{
    cmd.push_back(s);
    time.push_back(t);
    world.push_back(Vector2(x, y));
    relative.push_back(Vector2(rx, ry));
}

void readData(int id)
{
    stringstream ss;
    ss << id;
    string fileName = "data/" + name + "_" + ss.str() + ".txt";
    cout << fileName << endl;
    fstream fin;
    fin.open(fileName.c_str(), fstream::in);
    getline(fin, sentence[id]);
    fin >> mode[id];
    fin >> widthRatio[id] >> heightRatio[id];
    fin >> width[id] >> height[id];
    if (width[id] > 2 * height[id])
        scale[id] = "1x1";
    else
        scale[id] = "1x3";

    keyboardSize[id] = (widthRatio[id] / 0.8) + 0.25f;

    words.clear();
    string word = "";
    rep(i, sentence[id].length())
        if (sentence[id][i] >= 'a' && sentence[id][i] <= 'z')
            word += sentence[id][i];
        else
        {
            words.push_back(word);
            word = "";
        }
    if (word.length() > 0)
        words.push_back(word);
    double startTime = -1;
    cmd.clear(); time.clear();
    world.clear(); relative.clear();

    string s;
    double t, x, y, rx, ry, lastT;
    while (fin >> s)
    {
        fin >> t;
        if (s == "PhraseEnd")
        {
            linePushBack(s, t);
            break;
        }
        if (s == "Backspace")
        {
            startTime = -1;
            cmd.clear(); time.clear();
            world.clear(); relative.clear();
            //linePushBack(s, t);
            continue;
        }
        lastT = t;
        fin >> x >> y >> rx >> ry;
        linePushBack(s, t, x, y, rx, ry);
        if (s == "Began")
        {
            if (startTime == -1)
                startTime = t;
        }
    }
    fin.close();
}

bool outKeyboard(Vector2 v, float sc)
{
    return v.x > 0.5 || v.x < -0.5 || v.y > (0.5 * 0.3 * sc) || v.y < -(0.5 * 0.3 * sc);
}

void calcCandidate(int id)
{
    fstream& fout = freqFout;
    int line = 0, p = 0, q = 0, sc = 1;

    if (scale[id] == "1x3")
        p = 1, sc = 3;
    if (same(keyboardSize[id], 0.75))
        q = 1;
    else if (same(keyboardSize[id], 1))
        q = 2;
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
        cout << word << " ";
        wordCount[p][q]++;
        int num = SAMPLE_NUM;
        vector<Vector2> location = temporalSampling(wordToPath(word, sc), num);

        int l = 0, r = rawstroke.size() - 1;

        while (outKeyboard(rawstroke[l], sc) && l < r) l++;
        while (outKeyboard(rawstroke[r], sc) && l < r) r--;
        vector <Vector2> stroke_cut;
        FOR(i, l, r)
            stroke_cut.push_back(rawstroke[i]);
        vector<Vector2> stroke_c = temporalSampling(stroke_cut, num);

        double f = dict_map[word];
        if (f == 0) f = freq[LEXICON_SIZE - 1];
        result[0] = match(stroke_c, location, dtw, DTW);
        FOR(i, 6, THETA_NUM - 1)
            result[i] = exp(-0.5 * sqr(result[0] / (DELTA_T * i))) * f;
        rep(i, THETA_NUM)
            rk[i] = 1;
        rep(j, LEXICON_SIZE)
        {
            if (word == dict[j])
                continue;
            vector<Vector2>& location = dict_location[j][p];
            if (dist(location.back(), stroke_c.back()) > 0.3)
                continue;
            double disDTW = match(stroke_c, location, dtw, DTW);
            if (disDTW < result[0]) rk[0]++;
            FOR(i, 6, THETA_NUM - 1)
                if (rk[i] <=12 && exp(-0.5 * sqr(disDTW / (DELTA_T * i))) * freq[j] > result[i])
                    rk[i]++;
        }
        rep(i, THETA_NUM)
            if (rk[i] <= 12)
                rkCount[p][q][i][rk[i]]++;
        cout << rk[0] << ":" << dict_map[word] << endl;
    }
    cout << endl;
}

void outputCandidate()
{
    fstream& fout = freqFout;
    cout << endl;
    rep(p, 2)
    {
        string scale = (p==0)?"1x1":"1x3";
        rep(q, 3)
        {
            string keyboardSize = "0.75";
            if (q == 1)
                keyboardSize = "1.00";
            else if (q == 2)
                keyboardSize = "1.25";

            rep(i, THETA_NUM)
            {
                if (i < 6) continue;
                For(j, 12)
                    rkCount[p][q][i][j] += rkCount[p][q][i][j-1];
                fout<< userID << ","
                    << scale << ","
                    << keyboardSize << ","
                    << i * DELTA_T / 0.1;
                For(j, 12)
                    fout << "," << rkCount[p][q][i][j] / wordCount[p][q];
                fout << endl;
            }
        }
    }
}

void clean()
{
    memset(rkCount, 0, sizeof(rkCount));
    memset(wordCount, 0, sizeof(wordCount));
}

int main()
{
    init();
    FOR(p, USER_L - 1, USER_NUM - 1)
    {
        name = user[p];
        userID = id[p];
        rep(i, PHRASES)
        {
            readData(i);
            calcCandidate(i);
        }
        outputCandidate();
        clean();
    }
    freqFout.close();
    return 0;
}

