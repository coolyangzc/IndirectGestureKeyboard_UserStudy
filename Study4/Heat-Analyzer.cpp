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
double sx[2][26], sy[2][26], tx[2][26], ty[2][26], scnt[2][26], tcnt[2][26];
vector<Vector2> cor[2][26][2];
vector<string> who[2][26][2];


string userID;
fstream heatFout, centerFout, cleanFout;

void init()
{
    string heatFileName = "res/Heat_Study2.csv";
    heatFout.open(heatFileName.c_str(), fstream::out);
    heatFout << "id,usage,type,key,x,y" << endl;

    string centerFileName = "res/Heat_Center_Study2.csv";
    centerFout.open(centerFileName.c_str(), fstream::out);
    centerFout << "id,usage,type,key,x,y" << endl;

    cleanFout.open("res/Heat_Clean_Study2.csv", fstream::out);
    cleanFout << "id,usage,type,key,x,y" << endl;

    initKeyboard(dtw);
}

void calcHeat(int id)
{
    fstream& fout = heatFout;
    int line = 0, p = 0, sc = 3;

    if (mode[id] == "Direct")
        p = 1;

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
        scnt[p][sc] ++; tcnt[p][tc]++;

        cor[p][sc][0].push_back(Vector2(rawstroke.front().x, rawstroke.front().y));
        cor[p][tc][1].push_back(Vector2(rawstroke.back().x, rawstroke.back().y));
        who[p][sc][0].push_back(userID);
        who[p][tc][1].push_back(userID);

        sx[p][sc] += rawstroke.front().x;
        sy[p][sc] += rawstroke.front().y;

        tx[p][tc] += rawstroke.back().x;
        ty[p][tc] += rawstroke.back().y;

        string usage = (p==0)?"Indirect":"Direct";

        fout<< userID << ","
            << usage << ","
            << "start,"
            << word[0] << ","
            << rawstroke[0].x << ","
            << rawstroke[0].y << ","
            << endl;

        fout<< userID << ","
            << usage << ","
            << "end,"
            << word[word.length()-1] << ","
            << rawstroke.back().x << ","
            << rawstroke.back().y << ","
            << endl;
    }
}

void calcUserHeat()
{
    rep(p, 2)
        rep(i, 26)
        {
            sx[p][i] /= scnt[p][i];
            sy[p][i] /= scnt[p][i];
            tx[p][i] /= tcnt[p][i];
            ty[p][i] /= tcnt[p][i];
            string usage = (p==0)?"Indirect":"Direct";
            if (scnt[p][i] > 0)
                centerFout  << userID << ","
                            << usage << ","
                            << "start,"
                            << (char)(i + 'a') << ","
                            << sx[p][i] << ","
                            << sy[p][i]
                            << endl;

            if (tcnt[p][i] > 0)
                centerFout  << userID << ","
                            << usage << ","
                            << "end,"
                            << (char)(i + 'a') << ","
                            << tx[p][i] << ","
                            << ty[p][i]
                            << endl;
        }
    memset(sx, 0, sizeof(sx));
    memset(tx, 0, sizeof(tx));
    memset(sy, 0, sizeof(sy));
    memset(ty, 0, sizeof(ty));
    memset(scnt, 0, sizeof(scnt));
    memset(tcnt, 0, sizeof(tcnt));
}

void calcCleanHeat()
{
    int tot = 0, used = 0;
    rep(p, 2)
        rep(c, 26)
            rep(t, 2)
            {
                vector<Vector2>& v = cor[p][c][t];
                int n = v.size();
                if (n <= 15)
                    continue;
                double x = 0, y = 0, sx = 0, sy = 0;
                rep(i, n)
                {
                    x += v[i].x; y += v[i].y;
                }
                x /= n; y /= n;

                rep(i, n)
                {
                    sx += sqr(v[i].x - x);
                    sy += sqr(v[i].y - y);
                }
                sx = sqrt(sx / (n - 1));
                sy = sqrt(sy / (n - 1));
                string usage = (p==0)?"Indirect":"Direct";
                string type = (t==0)?"start":"end";
                cout << usage << "," << type << "," << (char)(c + 'a') << "," << n
                                   << "," << x << "," << y << "," << sx << "," << sy << endl;
                rep(i, n)
                    if (abs(v[i].x - x) < 3 * sx && abs(v[i].y - y) < 3 * sy)
                    {
                        used++;
                        cleanFout << who[p][c][t][i] << "," << usage << "," << type << "," << (char)(c + 'a')
                                  << "," << v[i].x << "," << v[i].y << endl;
                    }
                tot += n;
            }
    cout << used << "/" << tot << endl;
}

stringstream ss;

int main()
{
    init();
    FOR(u, USER_L - 1, USER_NUM - 1)
    {
        ss.clear();ss.str("");
        ss << u + 1;
        userID = ss.str();
        rep(i, 40)
        {
            ss.clear();ss.str("");
            ss << i;
            string fileName = "data/" + user[u] + "_" + ss.str() + ".txt";
            readData(fileName, i);
            calcHeat(i);
        }

        FOR(i, 40, PHRASES - 1)
        {
            ss.clear();ss.str("");
            ss << i - 39;
            string fileName = "data/Direct/" + user[u] + "_" + ss.str() + ".txt";
            readData(fileName, i);
            calcHeat(i);
        }
        calcUserHeat();
    }
    calcCleanHeat();
    rep(i, 26)
        printf("%c: %d, %.4lf%%; %d, %.4lf%%\n", i + 'a',
               s[i], (double)s[i] / tot * 100, t[i], (double)t[i] / tot * 100);
    heatFout.close();
    cleanFout.close();
    centerFout.close();
    return 0;
}

