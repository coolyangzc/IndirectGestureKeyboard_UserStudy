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

double dtw[MAXSAMPLE][MAXSAMPLE];
int s[26], t[26], tot;
double sx[2][3][26], sy[2][3][26], tx[2][3][26], ty[2][3][26], scnt[2][3][26], tcnt[2][3][26];
string userID;
fstream heatFout, centerFout;

void init()
{
    string heatFileName = "res/Heat_Study1.csv";
    heatFout.open(heatFileName.c_str(), fstream::out);
    heatFout << "id,scale,size,type,key,x,y" << endl;

    string centerFileName = "res/Heat_Center_Study1.csv";
    centerFout.open(centerFileName.c_str(), fstream::out);
    centerFout << "id,scale,size,type,key,x,y" << endl;

    initKeyboard(dtw);
}

void calcHeat(int id)
{
    fstream& fout = heatFout;
    int line = 0, p = 0, q = 0, sc = 1;

    if (scale[id] == "1x3")
        p = 1, sc = 3;
    if (same(keyboardSize[id], 1))
        q = 1;
    else if (same(keyboardSize[id], 1.25))
        q = 2;
    rep(w, words.size())
    {
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
        string word = words[w];
        char sc = word[0] - 'a', tc = word[word.length()-1] - 'a';
        tot++;
        s[sc]++; t[tc]++;
        scnt[p][q][sc] ++; tcnt[p][q][tc]++;

        sx[p][q][sc] += rawstroke.front().x;
        sy[p][q][sc] += rawstroke.front().y;

        tx[p][q][tc] += rawstroke.back().x;
        ty[p][q][tc] += rawstroke.back().y;

        fout<< userID << ","
            << scale[id] << ","
            << keyboardSize[id] << ",";
        fout<< "start,"
            << word[0] << ","
            << rawstroke[0].x << ","
            << rawstroke[0].y << ","
            << endl;

        fout<< userID << ","
            << scale[id] << ","
            << keyboardSize[id] << ",";
        fout<< "end,"
            << word[word.length()-1] << ","
            << rawstroke.back().x << ","
            << rawstroke.back().y << ","
            << endl;
    }
}

int main()
{
    init();
    FOR(u, USER_L - 1, USER_NUM - 1)
    {
        userID = id[u];
        rep(i, PHRASES)
        {
            readData(user[u], i);
            calcHeat(i);
        }
        rep(p, 2) rep(q, 3)
            rep(i, 26)
            {
                sx[p][q][i] /= scnt[p][q][i];
                sy[p][q][i] /= scnt[p][q][i];
                tx[p][q][i] /= tcnt[p][q][i];
                ty[p][q][i] /= tcnt[p][q][i];
                string scale = (p==0)?"1x1":"1x3";
                string keyboardSize = "0.75";
                if (q == 1)
                    keyboardSize = "1.0";
                else if (q == 2)
                    keyboardSize = "1.25";
                if (scnt[p][q][i] > 0)
                    centerFout  << userID << ","
                                << scale << ","
                                << keyboardSize << ","
                                << "start,"
                                << (char)(i + 'a') << ","
                                << sx[p][q][i] << ","
                                << sy[p][q][i]
                                << endl;

                if (tcnt[p][q][i] > 0)
                    centerFout  << userID << ","
                                << scale << ","
                                << keyboardSize << ","
                                << "end,"
                                << (char)(i + 'a') << ","
                                << tx[p][q][i] << ","
                                << ty[p][q][i]
                                << endl;
            }
        memset(sx, 0, sizeof(sx));
        memset(tx, 0, sizeof(tx));
        memset(sy, 0, sizeof(sy));
        memset(ty, 0, sizeof(ty));
        memset(scnt, 0, sizeof(scnt));
        memset(tcnt, 0, sizeof(tcnt));
    }
    rep(i, 26)
        printf("%c: %d, %.4lf%%; %d, %.4lf%%\n", i + 'a',
               s[i], (double)s[i] / tot * 100, t[i], (double)t[i] / tot * 100);
    heatFout.close();
    centerFout.close();
    return 0;
}

