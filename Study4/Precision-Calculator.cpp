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

const int SAMPLE_NUM = 32;

fstream eachPrecisionFout, avgPrecisionFout;

double dtw[MAXSAMPLE][MAXSAMPLE];

string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];
map<string, int> dict_map;
map<string, double> katz_alpha;
map<string, ll> unigram_map;
map<pair<string, string>, double> bigram_map;

vector<double> precision[USER_NUM][PHRASES];
int words_num[USER_NUM][2];
double sum_precision[USER_NUM][2];

void initLexicon()
{
    fstream fin;
    fin.open("ANC-written-noduplicate+pangram.txt", fstream::in);
    string s;
    rep(i, LEXICON_SIZE)
    {
        fin >> dict[i] >> freq[i];
        dict_map[dict[i]] = freq[i];
    }
    fin.close();
}

void init()
{
    initKeyboard(dtw);
    initLexicon();
}

void calcPhrasePrecision(int user_id, int phrase_id)
{
    int line = 0, p = 0, sc = 3;

    if (mode[phrase_id] == "Direct")
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

        precision[user_id][phrase_id].push_back(match(stroke, location, dtw, DTW) * 10); // converse to key width by * 10
    }
}

void calcUserGesturePrecision(int p)
{
    fstream& fout = avgPrecisionFout;

    stringstream ss;
    ss.clear(); ss.str("");
    ss << p + 1;
    string userID = ss.str();
    rep(i, 40)
    {
        ss.clear(); ss.str("");
        ss << i;
        string fileName = "data/" + user[p] + "_" + ss.str() + ".txt";
        readData(fileName, i);
        calcPhrasePrecision(p, i);
    }

    FOR(i, 40, PHRASES - 1)
    {
        ss.clear(); ss.str("");
        ss << i - 39;
        string fileName = "data/Direct/" + user[p] + "_" + ss.str() + ".txt";
        readData(fileName, i);
        calcPhrasePrecision(p, i);
    }
}

void outputResults()
{
    string eachFileName = "res/precision_of_each_gesture.csv";
    eachPrecisionFout.open(eachFileName.c_str(), fstream::out);
    eachPrecisionFout << "user_id,usage,phrase_id,word_id,precision" << endl;

    FOR(user, USER_L - 1, USER_NUM - 1)
        rep(p, PHRASES)
        {
            int cur_mode = (p < 40) ? 0 : 1;
            rep(w, precision[user][p].size())
            {
                double& pre = precision[user][p][w];
                words_num[user][cur_mode] ++;
                sum_precision[user][cur_mode] += pre;
                eachPrecisionFout << user + 1 << "," << ((cur_mode == 0) ? "Indirect," : "Direct,");
                eachPrecisionFout << p + 1 << "," << w + 1 << "," << pre << endl;
            }
        }
    eachPrecisionFout.close();

    string avgFileName = "res/average_precision_of_users.csv";
    avgPrecisionFout.open(avgFileName.c_str(), fstream::out);
    avgPrecisionFout << "user_id,Direct,Indirect" << endl;

    FOR(user, USER_L - 1, USER_NUM - 1)
    {
        avgPrecisionFout << user + 1 << ","
        << sum_precision[user][1] / words_num[user][1] << ","
        << sum_precision[user][0] / words_num[user][0] << endl;
    }
    avgPrecisionFout.close();
}

int main()
{
    init();
    FOR(p, USER_L - 1, USER_NUM - 1)
        calcUserGesturePrecision(p);
    outputResults();
}
