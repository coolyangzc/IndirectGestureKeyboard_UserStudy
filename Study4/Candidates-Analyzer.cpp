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

const int ALG_NUM = 6; //SHARK2(no, unigram, bigram), DTW(no, unigram, bigram)
int rk[ALG_NUM];
double result[ALG_NUM];
double rkCount[2][ALG_NUM][RANK + 1]; //[type][std;dtw;dtw*freq][rank]

const int SAMPLE_NUM = 32;

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
    string candFileName = "res/Candidate_Study2.csv";
    candFout.open(candFileName.c_str(), fstream::out);
    candFout << "id,usage,algorithm,top1,top2,top3,top4,top5,top6,top7,top8,top9,top10,top11,top12,top13" << endl;
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
    fstream& fout = candFout;
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

        int num = SAMPLE_NUM;
        vector<Vector2> location = temporalSampling(wordToPath(word, sc), num);
        vector<Vector2> stroke = temporalSampling(rawstroke, num);

        wordCount[p]++;
        double f = dict_map[word], bi_f = calcBigramProb(w);
        if (f == 0)
        {
            f = freq[LEXICON_SIZE - 1];
            cout << "Warning: OOV!" << endl;
        }
        result[0] = match(stroke, location, dtw, Standard);
        result[1] = exp(-0.5 * sqr(result[0] / 0.025)) * f;
        result[2] = bi_f - 80 * result[0];

        result[3] = match(stroke, location, dtw, DTW);
        result[4] = exp(-0.5 * sqr(result[3] / 0.025)) * f;
        result[5] = bi_f - 80 * result[3];

        rep(alg, ALG_NUM)
            rk[alg] = 1;
        rep(j, LEXICON_SIZE)
        {
            if (word == dict[j])
                continue;
            vector<Vector2>& location = dict_location[j][1];
            if (dist(location.back(), stroke.back()) > 0.3)
                continue;
            double disSHARK = match(stroke, location, dtw, Standard);
            double disDTW = match(stroke, location, dtw, DTW);
            bi_f = calcBigramProb(w, j);
            rep(alg, ALG_NUM)
            {
                if (rk[alg] > RANK)
                    continue;
                switch(alg)
                {
                case 0:
                    if (disSHARK < result[alg])
                        rk[alg]++;
                    break;
                case 1:
                    if ((exp(-0.5 * sqr(disSHARK / 0.025)) * freq[j]) > result[alg])
                        rk[alg]++;
                    break;
                case 2:
                    if (bi_f - 80 * disSHARK > result[alg])
                        rk[alg]++;
                    break;
                case 3:
                    if (disDTW < result[alg])
                        rk[alg]++;
                    break;
                case 4:
                    if ((exp(-0.5 * sqr(disDTW / 0.025)) * freq[j]) > result[alg])
                        rk[alg]++;
                    break;
                case 5:
                    if (bi_f - 80 * disDTW > result[alg])
                        rk[alg]++;
                    break;
                }
            }
        }
        rep(i, ALG_NUM)
            if (rk[i] <= RANK)
                rkCount[p][i][rk[i]]++;
    }
}

void outputCandidate()
{
    fstream& fout = candFout;
    rep(p, 2)
    {
        string usage = (p==0)?"Indirect":"Direct";
        rep(alg, ALG_NUM)
        {
            For(j, RANK)
                rkCount[p][alg][j] += rkCount[p][alg][j-1];
            fout<< userID << ","
                << usage << ",";
            if (alg == 0) fout << "SHARK2";
            if (alg == 1) fout << "SHARK2(Unigram)";
            if (alg == 2) fout << "SHARK2(Bigram)";
            if (alg == 3) fout << "DTW";
            if (alg == 4) fout << "DTW(Unigram)";
            if (alg == 5) fout << "DTW(Bigram)";
            For(j, RANK)
                fout << "," << rkCount[p][alg][j] / wordCount[p];
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
    candFout.close();
    return 0;
}

