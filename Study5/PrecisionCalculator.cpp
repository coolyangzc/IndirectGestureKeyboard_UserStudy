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

double dtw[MAXSAMPLE][MAXSAMPLE];

// 0: Basic; 1: FixStart
double sum_precision[USER_NUM + 1][2];
int gesture_num[USER_NUM + 1][2];

void calcPrecision(int user, int id)
{
    vector<string> candidates;
    int wordP = -1;

    rep(i, span.size())
    {
        if (span[i].type == Delete)
            wordP = max(wordP - 1, -1);
        else if (span[i].type == Cancel)
        {
            if (candMethod[id] == "Radial")
                wordP--;
        }
        else if (span[i].type == Gesture)
        {
            sentenceToWords(span[i].para, candidates);
            wordP++;
            if (wordP >= words.size())
                continue;

            int same = -1;
            rep(j, candidates.size())
                if (candidates[j] == words[wordP])
                {
                    same = j;
                    break;
                }

            vector<Vector2> p; p.clear();
            FOR(k, span[i].startLine + 1, span[i].endLine)
                if (cmd[k] == "Began" || cmd[k] == "Moved" || cmd[k] == "Stationary" || cmd[k] == "Ended")
                {
                    Vector2 cur(relative[k].x, relative[k].y * 0.3 * 3);
                    if (p.empty() || dist(cur, p.back()) > 1e-8)
                        p.push_back(cur);
                }

            double len = 0, lenWord = 0;
            if (p.size() > 1)
                rep(k, p.size() - 1)
                    len += dist(p[k], p[k+1]);
            string word = ((mode[id] == "FixStart")?"g":"") + words[wordP];

            vector<Vector2> v = wordToPath(word, 3);
            rep(k, v.size() - 1)
                lenWord += dist(v[k], v[k+1]);
            //Length Check for Clean
            if (len < eps || lenWord < eps)
                continue;
            if (same == -1)
                if (len > lenWord * 2 + 0.1 || lenWord > len * 2 + 0.1)
                    continue;
            if (mode[id] == "FixStart")
            {
                For(i, p.size() - 1)
                {
                    p[i].x -= p[0].x;
                    p[i].y -= p[0].y;
                }
                p[0].x = p[0].y = 0;
            }
            p = temporalSampling(p, SAMPLE_NUM);
            v = temporalSampling(v, SAMPLE_NUM);
            int m = (mode[id] == "FixStart") ? 1 : 0;

            sum_precision[user][m] += match(p, v, dtw, DTW) * 10; // converse to key width by * 10
            gesture_num[user][m] ++;
        }
    }
}

int main()
{
    calcKeyLayout();
    initDTW(dtw);
    fstream fout;
    string fileName = "res/Precision.csv";
    fout.open(fileName.c_str(), fstream::out);
    fout << "user_id,Basic_precision,FixStart_precision" << endl;

    stringstream ss;
    FOR(p, USER_L, USER_NUM)
    {
        rep(i, 80)
        {
            ss.clear();ss.str("");
            ss << i;
            string fileName = "data/" + user[p] + "_" + ss.str() + ".txt";
            readData(fileName, i);
            calcTimeSpan(i);
            calcPrecision(p, i);
        }
        fout << p << ","
            << sum_precision[p][0] / gesture_num[p][0] << ","
            << sum_precision[p][1] / gesture_num[p][1] << endl;
    }
    fout.close();
    return 0;
}
