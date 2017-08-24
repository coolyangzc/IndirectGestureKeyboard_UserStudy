#include "Common.h"
#include "Vector2.h"

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

const int ALG_NUM = 6;
int rk[ALG_NUM];
double result[ALG_NUM];
double rkCount[2][3][ALG_NUM][13]; //[scale][size][std;dtw;dtw*freq][rank]

const int SAMPLE_NUM = 32;

double dtw[MAXSAMPLE][MAXSAMPLE];

string sentence[PHRASES], mode[PHRASES], scale[PHRASES];
double height[PHRASES], width[PHRASES], heightRatio[PHRASES], widthRatio[PHRASES], keyboardSize[PHRASES];
Vector2 keyPos[26];

int wordCount[2][3]; //[scale][size]


string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];
map<string, int> dict_map;
vector<Vector2> dict_location[LEXICON_SIZE][2]; //[scale: 0 for 1x1, 1 for 1x3]

vector<string> cmd;
vector<string> words;

vector<double> time;
vector<Vector2> world, relative;

string name, userID, candFileName;
fstream candFout;

void initDTW()
{
    rep(i, MAXSAMPLE)
        rep(j, MAXSAMPLE)
            dtw[i][j] = inf;
    dtw[0][0] = 0;
}

vector<Vector2> wordToPath(string word, float scale)
{
    vector<Vector2> pts;
    int preKey = -1;
    rep(i, word.length())
    {
        int key = word[i] - 'a';
        if (key != preKey)
            pts.push_back(Vector2(keyPos[key].x, keyPos[key].y * 0.3 * scale));
        preKey = key;
    }
    return pts;
}

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

void calcKeyLayout()
{
    string line1 = "qwertyuiop";
    string line2 = "asdfghjkl";
    string line3 = "zxcvbnm";
    rep(i, line1.length())
    {
        keyPos[line1[i] - 'a'] = Vector2(-0.45 + i * 0.1, 0.3333);
    }
    rep(i, line2.length())
    {
        keyPos[line2[i] - 'a'] = Vector2(-0.4 + i * 0.1, 0);
    }
    rep(i, line3.length())
    {
        keyPos[line3[i] - 'a'] = Vector2(-0.35 + i * 0.1, -0.333);
    }
}

void init()
{
    candFileName = "res/Candidate_Study1.csv";
    candFout.open(candFileName.c_str(), fstream::out);
    candFout << "id,scale,size,algorithm,top1,top2,top3,top4,top5,top6,top7,top8,top9,top10,top11,top12" << endl;
    initDTW();
    calcKeyLayout();
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
    int alpha = sentence[id].length();
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
            linePushBack(s, t);
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
    fstream& fout = candFout;
    int line = 0, p = 0, q = 0, sc = 1;

    if (scale[id] == "1x3")
        p = 1, sc = 3;
    if (same(keyboardSize[id], 1))
        q = 1;
    else if (same(keyboardSize[id], 1.25))
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
        vector<Vector2> stroke = temporalSampling(rawstroke, num);

        int l = 0, r = rawstroke.size() - 1;

        while (outKeyboard(rawstroke[l], sc) && l < r) l++;
        while (outKeyboard(rawstroke[r], sc) && l < r) r--;
        vector <Vector2> stroke_cut;
        if (l < r)
            FOR(i, l, r)
                stroke_cut.push_back(rawstroke[i]);
        else
            stroke_cut = rawstroke;
        vector<Vector2> stroke_c = temporalSampling(stroke_cut, num);

        result[0] = match(stroke, location, dtw, Standard);
        result[1] = match(stroke, location, dtw, DTW);
        double f = dict_map[word];
        if (f == 0) f = freq[LEXICON_SIZE - 1];
        result[2] = exp(-0.5 * sqr(result[0] / 0.015)) * f;
        result[3] = exp(-0.5 * sqr(result[1] / 0.015)) * f;
        result[4] = match(stroke_c, location, dtw, Standard);
        result[5] = match(stroke_c, location, dtw, DTW);

        rep(alg, ALG_NUM)
            rk[alg] = 1;
        rep(j, LEXICON_SIZE)
        {
            if (word == dict[j])
                continue;
            vector<Vector2>& location = dict_location[j][p];
            if (dist(location.back(), stroke.back()) > 0.3)
                continue;
            double disSHARK = match(stroke, location, dtw, Standard);
            double disDTW = match(stroke, location, dtw, DTW);
            rep(alg, ALG_NUM)
            {
                if (rk[alg] > 12)
                    continue;
                switch(alg)
                {
                case 0:
                    if (disSHARK < result[alg])
                        rk[alg]++;
                    break;
                case 1:
                    if (disDTW < result[alg])
                        rk[alg]++;
                    break;
                case 2:
                    if ((exp(-0.5 * sqr(disSHARK / 0.015)) * freq[j]) > result[alg])
                        rk[alg]++;
                    break;
                case 3:
                    if ((exp(-0.5 * sqr(disDTW / 0.015)) * freq[j]) > result[alg])
                        rk[alg]++;
                    break;
                case 4:
                    if (match(stroke_c, location, dtw, Standard, result[alg]) < result[alg])
                        rk[alg]++;
                    break;
                case 5:
                    if (match(stroke_c, location, dtw, DTW, result[alg]) < result[alg])
                        rk[alg]++;
                    break;
                }
            }
        }
        rep(i, ALG_NUM)
            if (rk[i] <= 12)
                rkCount[p][q][i][rk[i]]++;
        cout << rk[3] << ":" << dict_map[word] << endl;
    }
    cout << endl;
}

void outputCandidate()
{
    fstream& fout = candFout;
    cout << endl;
    rep(p, 2)
    {
        string scale = (p==0)?"1x1":"1x3";
        rep(q, 3)
        {
            string keyboardSize = "0.75";
            if (q == 1)
                keyboardSize = "1.0";
            else if (q == 2)
                keyboardSize = "1.25";

            rep(alg, ALG_NUM)
            {
                For(j, 12)
                    rkCount[p][q][alg][j] += rkCount[p][q][alg][j-1];
                fout<< userID << ","
                    << scale << ","
                    << keyboardSize << ",";
                if (alg == 0) fout << "SHARK2";
                if (alg == 1) fout << "DTW";
                if (alg == 2) fout << "SHARK2*freq";
                if (alg == 3) fout << "DTW*freq";
                if (alg == 4) fout << "SHARK2(cut)";
                if (alg == 5) fout << "DTW(cut)";
                For(j, 12)
                    fout << "," << rkCount[p][q][alg][j] / wordCount[p][q];
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
    candFout.close();
    return 0;
}

