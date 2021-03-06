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
const bool TEST_SHAPE_ONLY = false;

const int ALG_NUM = 6;
int rk[ALG_NUM];
double result[ALG_NUM];
double rkCount[2][3][ALG_NUM][13]; //[scale][size][std;dtw;dtw*freq][rank]

const int SAMPLE_NUM = 32;

double dtw[MAXSAMPLE][MAXSAMPLE];

int wordCount[2][3]; //[scale][size]


string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];
map<string, int> dict_map;
vector<Vector2> dict_location[LEXICON_SIZE][2], dict_shape[LEXICON_SIZE][2]; //[scale: 0 for 1x1, 1 for 1x3]

string userID;
fstream candFout;

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
}

void init()
{
    string candFileName = "res/Candidate_Study1.csv";
    candFout.open(candFileName.c_str(), fstream::out);
    candFout << "id,scale,size,algorithm,top1,top2,top3,top4,top5,top6,top7,top8,top9,top10,top11,top12" << endl;
    initKeyboard(dtw);
    initLexicon();
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
        wordCount[p][q]++;
        int num = SAMPLE_NUM;
        vector<Vector2> location = temporalSampling(wordToPath(word, sc), num);
        vector<Vector2> stroke = temporalSampling(rawstroke, num);

        if (TEST_SHAPE_ONLY)
        {
            result[0] = match(stroke, location, dtw, Standard);
            vector<Vector2> norm_stroke = normalize(stroke);
            vector<Vector2> shape = normalize(location);
            result[1] = match(norm_stroke, shape, dtw, Standard);
            result[2] = exp(-0.5 * sqr(result[0] / 0.025)) * exp(-0.5 * sqr(result[1] / 0.025));
            rep(alg, 3)
                rk[alg] = 1;
            rep(j, LEXICON_SIZE)
            {
                if (word == dict[j])
                    continue;
                vector<Vector2>& location = dict_location[j][p];
                vector<Vector2>& shape = dict_shape[j][p];
                if (dist(location.back(), stroke.back()) > 0.3)
                    continue;
                double disLocation = match(stroke, location, dtw, Standard);
                double disShape = match(norm_stroke, shape, dtw, Standard);
                double disIntegrate = exp(-0.5 * sqr(disLocation / 0.025)) * exp(-0.5 * sqr(disShape / 0.025));
                rep(alg, 3)
                {
                    if (rk[alg] > 12)
                        continue;
                    switch(alg)
                    {
                    case 0:
                        if (disLocation < result[alg])
                            rk[alg]++;
                        break;
                    case 1:
                        if (disShape < result[alg])
                            rk[alg]++;
                        break;
                    case 2:
                        if (disIntegrate > result[alg])
                            rk[alg]++;
                        break;
                    }
                }
            }
            rep(i, 3)
                if (rk[i] <= 12)
                    rkCount[p][q][i][rk[i]]++;
            continue;
        }

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
        result[2] = exp(-0.5 * sqr(result[0] / 0.025)) * f;
        result[3] = exp(-0.5 * sqr(result[1] / 0.025)) * f;
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
                    if ((exp(-0.5 * sqr(disSHARK / 0.025)) * freq[j]) > result[alg])
                        rk[alg]++;
                    break;
                case 3:
                    if ((exp(-0.5 * sqr(disDTW / 0.025)) * freq[j]) > result[alg])
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
    }
}

void outputCandidate()
{
    fstream& fout = candFout;
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
            if (TEST_SHAPE_ONLY)
            {
                rep(alg, 3)
                {
                    For(j, 12)
                        rkCount[p][q][alg][j] += rkCount[p][q][alg][j-1];
                    fout<< userID << ","
                        << scale << ","
                        << keyboardSize << ",";
                    if (alg == 0) fout << "Location";
                    if (alg == 1) fout << "Shape";
                    if (alg == 2) fout << "Location*Shape";
                    For(j, 12)
                        fout << "," << rkCount[p][q][alg][j] / wordCount[p][q];
                    fout << endl;
                }
                continue;
            }
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
        userID = id[p];
        rep(i, PHRASES)
        {
            readData(user[p], i);
            calcCandidate(i);
        }
        outputCandidate();
        clean();
    }
    candFout.close();
    return 0;
}

