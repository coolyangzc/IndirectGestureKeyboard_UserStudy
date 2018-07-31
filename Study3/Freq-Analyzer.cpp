#include "Keyboard.h"
#include "DataReader.h"

#include <map>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;

const int USER_L = 1;
const bool CUT_OUTKEYBORAD_PART = false;
const bool TEST_UNIGRAM_FREQ = false;
const bool SKIP_PANGRAMS = false;
const bool TEST_BIGRAM_FREQ = true;
const bool USE_KATZ_SMOOTHING = true;

const int THETA_NUM = 100, GAMMA_NUM = 400;
const double DELTA_T = 0.0005, DELTA_G = 1;

const int SAMPLE_NUM = 32;

double bi_eps, uni_tot_freq;

int rk[THETA_NUM + GAMMA_NUM];
double result[THETA_NUM + GAMMA_NUM];
double rkCount[2][3][THETA_NUM + GAMMA_NUM][13]; //[scale][size][][rank]

double dtw[MAXSAMPLE][MAXSAMPLE];

int wordCount[2][3]; //[scale][size]

string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];
map<string, int> dict_map;
map<string, double> katz_alpha;
map<pair<string, string>, double> bigram_map;
vector<Vector2> dict_location[LEXICON_SIZE][2]; //[scale: 0 for 1x1, 1 for 1x3]

string  userID;
fstream freqFout, bigramFout;

void initLexicon()
{
    fstream fin;
    if (USE_KATZ_SMOOTHING)
        fin.open("unigrams-written.txt", fstream::in);
    else
        fin.open("ANC-written-noduplicate+pangram.txt", fstream::in);


    rep(i, LEXICON_SIZE)
    {
        fin >> dict[i] >> freq[i];
        uni_tot_freq += freq[i];
        dict_map[dict[i]] = freq[i];
        dict_location[i][0] = temporalSampling(wordToPath(dict[i], 1), SAMPLE_NUM);
        dict_location[i][1] = temporalSampling(wordToPath(dict[i], 3), SAMPLE_NUM);
    }
    fin.close();

    if (TEST_BIGRAM_FREQ)
    {
        string s1, s2;
        double prob;
        if (USE_KATZ_SMOOTHING)
        {
            fin.open("bigrams-written-katz.txt", fstream::in);
            int bigrams_num;
            fin >> bigrams_num;
            rep(i, bigrams_num)
            {
                fin >> s1 >> s2 >> prob;
                bigram_map[mk(s1, s2)] = prob;
            }
            rep(i, LEXICON_SIZE)
            {
                fin >> s1 >> prob;
                katz_alpha[s1] = prob;
            }
        }
        else
        {
            fin.open("bigrams-written-prob.txt", fstream::in);
            fin >> s1 >> bi_eps;
            while (!fin.eof())
            {
                fin >> s1 >> s2 >> prob;
                bigram_map[mk(s1, s2)] = prob;
            }
        }
        fin.close();
    }

}

void init()
{
    if (TEST_UNIGRAM_FREQ)
    {
        string freqFileName = "res/Freq_Study1.csv";
        freqFout.open(freqFileName.c_str(), fstream::out);
        freqFout << "id,scale,size,theta(keywidth),top1,top2,top3,top4,top5,top6,top7,top8,top9,top10,top11,top12" << endl;
    }
    if (TEST_BIGRAM_FREQ)
    {
        string freqFileName = "res/Bigram_Freq_Study1.csv";
        if (USE_KATZ_SMOOTHING)
            freqFileName = "res/Bigram_Katz_Freq_Study1.csv";
        bigramFout.open(freqFileName.c_str(), fstream::out);
        bigramFout << "id,scale,size,gamma,top1,top2,top3,top4,top5,top6,top7,top8,top9,top10,top11,top12" << endl;
    }
    initKeyboard(dtw);
    initLexicon();
}

bool outKeyboard(Vector2 v, float sc)
{
    return v.x > 0.5 || v.x < -0.5 || v.y > (0.5 * 0.3 * sc) || v.y < -(0.5 * 0.3 * sc);
}

double calcBigramProb(double freq, int w, int now_id = -1)
{
    double bi_f;
    if (w)
    {
        string now = (now_id == -1)?words[w]:dict[now_id];
        bi_f = bigram_map[mk(words[w - 1], now)];
        if (bi_f == 0)
            if (USE_KATZ_SMOOTHING)
                bi_f = katz_alpha[words[w - 1]] * freq;
            else
                bi_f = bi_eps / (dict_map[words[w - 1]] + LEXICON_SIZE * bi_eps);
    }
    else
        bi_f = freq / uni_tot_freq;
    return log(bi_f);
}

void calcCandidate(int id)
{
    if (SKIP_PANGRAMS)
    {
        if (sentence[id] == "the quick brown fox jumps over the lazy dog" ||
            sentence[id] == "the five boxing wizards jump quickly")
            return;
    }
    fstream& fout = freqFout;
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

        vector<Vector2> stroke_c;
        if (CUT_OUTKEYBORAD_PART)
        {
            int l = 0, r = rawstroke.size() - 1;
            while (outKeyboard(rawstroke[l], sc) && l < r) l++;
            while (outKeyboard(rawstroke[r], sc) && l < r) r--;
            vector <Vector2> stroke_cut;
            FOR(i, l, r)
                stroke_cut.push_back(rawstroke[i]);
            stroke_c = temporalSampling(stroke_cut, num);
        }
        else
            stroke_c = temporalSampling(rawstroke, num);

        double f = dict_map[word], bi_f = 0;
        if (f == 0)
            f = freq[LEXICON_SIZE - 1];

        if (TEST_BIGRAM_FREQ)
            bi_f = calcBigramProb(f, w);

        result[0] = match(stroke_c, location, dtw, DTW);

        rep(i, THETA_NUM + GAMMA_NUM)
            rk[i] = 1;

        if (TEST_UNIGRAM_FREQ)
            FOR(i, 6, THETA_NUM - 1)
                result[i] = exp(-0.5 * sqr(result[0] / (DELTA_T * i))) * f;
        if (TEST_BIGRAM_FREQ)
            FOR(i, 6, GAMMA_NUM - 1)
                result[i + THETA_NUM] = bi_f - (i * DELTA_G) * result[0];

        rep(j, LEXICON_SIZE)
        {
            if (word == dict[j])
                continue;
            vector<Vector2>& location = dict_location[j][p];
            if (dist(location.back(), stroke_c.back()) > 0.3)
                continue;
            double disDTW = match(stroke_c, location, dtw, DTW);
            if (disDTW < result[0]) rk[0]++;

            if (TEST_UNIGRAM_FREQ)
                FOR(i, 6, THETA_NUM - 1)
                    if (rk[i] <=12 && exp(-0.5 * sqr(disDTW / (DELTA_T * i))) * freq[j] > result[i])
                        rk[i]++;
            if (TEST_BIGRAM_FREQ)
            {
                bi_f = calcBigramProb(freq[j], w, j);
                FOR(i, 6, GAMMA_NUM - 1)
                    if (rk[i + THETA_NUM] <= 12 && bi_f - (i * DELTA_G) * disDTW > result[i + THETA_NUM])
                        rk[i + THETA_NUM] ++;
            }

        }
        rep(i, THETA_NUM + GAMMA_NUM)
            if (rk[i] <= 12)
                rkCount[p][q][i][rk[i]]++;
    }
}

void outputCandidate()
{
    fstream& fout = freqFout;
    fstream& biFout = bigramFout;
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
            if (TEST_UNIGRAM_FREQ)
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
            if (TEST_BIGRAM_FREQ)
                FOR(i, THETA_NUM + 6, THETA_NUM + GAMMA_NUM - 1)
                {
                    For(j, 12)
                        rkCount[p][q][i][j] += rkCount[p][q][i][j-1];
                    biFout<< userID << ","
                        << scale << ","
                        << keyboardSize << ","
                        << (i - THETA_NUM) * DELTA_G;
                    For(j, 12)
                        biFout << "," << rkCount[p][q][i][j] / wordCount[p][q];
                    biFout << endl;
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
    freqFout.close();
    return 0;
}

