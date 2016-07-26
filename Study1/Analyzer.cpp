#include "Common.h"
#include "Vector2.h"

#include <cstdio>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

double dtw[MAXSAMPLE][MAXSAMPLE];

string sentence[PHRASES], mode[PHRASES], scale[PHRASES];
double height[PHRASES], width[PHRASES], heightRatio[PHRASES], widthRatio[PHRASES], keyboardSize[PHRASES];
double WPM[PHRASES];
Vector2 keyPos[128];

vector<string> cmd;
vector<string> words;
vector<double> time;
vector<Vector2> world, relative;

string name, userID, disFileName, candFileName, WPMFileName;
fstream disFout, WPMFout, candFout;

void InitFstream(string user, string id)
{
    name = user;
    userID = id;
    disFileName = "res/Distance_" + userID + ".csv";
    WPMFileName = "res/WPM_" + userID + ".csv";
    candFileName = "res/Candidate_" + userID + ".csv";

    disFout.open(disFileName.c_str(), fstream::out);
    WPMFout.open(WPMFileName.c_str(), fstream::out);
    disFout << "id,scale,size,word,algorithm,sampleNum,coor,distance" << endl;
}

void InitDTW()
{
    rep(i, MAXSAMPLE)
        rep(j, MAXSAMPLE)
            dtw[i][j] = inf;
    dtw[0][0] = 0;
}

void CalcKeyLayout()
{
    string line1 = "qwertyuiop";
    string line2 = "asdfghjkl";
    string line3 = "zxcvbnm";
    rep(i, line1.length())
    {
        keyPos[line1[i]] = Vector2(-0.45 + i * 0.1, 0.3333);
    }
    rep(i, line2.length())
    {
        keyPos[line2[i]] = Vector2(-0.4 + i * 0.1, 0);
    }
    rep(i, line3.length())
    {
        keyPos[line3[i]] = Vector2(-0.35 + i * 0.1, -0.333);
    }
}

void LinePushBack(string s, double t, double x = 0, double y = 0, double rx = 0, double ry = 0)
{
    cmd.push_back(s);
    time.push_back(t);

    world.push_back(Vector2(x, y));
    relative.push_back(Vector2(rx, ry));
}

void ReadData(int id)
{
    stringstream ss;
    ss << id;
    string fileName = "data/" + name + "_" + ss.str() + ".txt";
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

    keyboardSize[id] = widthRatio[id] / 0.8;

    words.clear();
    int alpha = 0;
    string word = "";
    rep(i, sentence[id].length())
        if (sentence[id][i] >= 'a' && sentence[id][i] <= 'z')
        {
            alpha++;
            word += sentence[id][i];
        }
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
    double t, x, y, rx, ry;
    while (fin >> s)
    {
        fin >> t;
        if (same(s, "Backspace"))
        {
            startTime = -1;
            cmd.clear(); time.clear();
            world.clear(); relative.clear();
            LinePushBack(s, t);
            continue;
        }
        if (same(s, "PhraseEnd"))
        {
            LinePushBack(s, t);
            continue;
        }
        fin >> x >> y >> rx >> ry;
        LinePushBack(s, t, x, y, rx, ry);
        if (same(s, "Began"))
        {
            if (startTime == -1)
                startTime = t;
        }
    }

    WPM[id] = alpha / (t - startTime) * 12;
    fin.close();
}

void CalcWPM()
{
    WPMFout << "id,scale,size,sentence,WPM" << endl;
    rep(i, PHRASES)
    {
        WPMFout << userID << ","
                << scale[i] << ","
                << keyboardSize[i] << ","
                << sentence[i] << ","
                << WPM[i] << endl;
    }
    WPMFout.close();
}

void CalcDistance(int id, vector<int>& sampleNums)
{
    fstream& fout = disFout;
    int line = 0;
    double keyWidth = width[id] / 10;
    rep(w, words.size())
    {
        string word = words[w];
        vector<Vector2> pts, rawstroke;
        if (word.length() == 1)
            continue;
        rep(i, word.length())
        {
            int key = word[i];
            pts.push_back(Vector2(keyPos[key].x * width[id], keyPos[key].y * height[id]));
        }
        while (line < cmd.size())
        {
            string s = cmd[line];
            Vector2 p(relative[line].x * width[id], relative[line].y * height[id]);
            line++;
            if (rawstroke.size() == 0 || dist(rawstroke[rawstroke.size()-1], p) > eps)
                rawstroke.push_back(p);
            if (same(s, "Ended"))
                break;
        }
        if (rawstroke.size() <= 1)
            return;
        rep(i, sampleNums.size())
        {
            vector<Vector2> location = temporalSampling(pts, sampleNums[i]);
            vector<Vector2> stroke = temporalSampling(rawstroke, sampleNums[i]);

            double result = match(location, stroke, dtw, Standard) / sampleNums[i];
            fout<< userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "Standard" << ","
                << sampleNums[i] << ","
                << "pixel" << ","
                << result << endl;
            fout<< userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "Standard" << ","
                << sampleNums[i] << ","
                << "keyWidth" << ","
                << result / keyWidth << endl;

            result = match(location, stroke, dtw, DTW) / sampleNums[i];

            fout<< userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "DTW" << ","
                << sampleNums[i] << ","
                << "pixel" << ","
                << result << endl;
            fout<< userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "DTW" << ","
                << sampleNums[i] << ","
                << "keyWidth" << ","
                << result / keyWidth << endl;
        }
    }
}

int main()
{
    InitFstream("yzc", "1");
    InitDTW();
    CalcKeyLayout();

    int sampleNums[] = {16, 32, 64, 128, 256};
    vector<int> sample(sampleNums, sampleNums + 5);
    rep(i, 60)
    {
        ReadData(i);
        CalcDistance(i, sample);
    }
    CalcWPM();
    return 0;
}

