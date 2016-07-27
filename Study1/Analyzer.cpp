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
Vector2 keyPos[26];

int wordCount[2][3]; //[scale][size]
double stdCount[2][3][10][11], dtwCount[2][3][10][11]; //[scale][size][num][rank];

string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];
vector<Vector2> path[LEXICON_SIZE];

vector<string> cmd;
vector<string> words;

vector<double> time;
vector<Vector2> world, relative;

string name, userID, disFileName, candFileName, WPMFileName;
fstream disFout, WPMFout, candFout;

void initFstream(string user, string id)
{
    name = user;
    userID = id;
    disFileName = "res/Distance_" + userID + ".csv";
    WPMFileName = "res/WPM_" + userID + ".csv";
    candFileName = "res/Candidate_" + userID + ".csv";

    disFout.open(disFileName.c_str(), fstream::out);
    WPMFout.open(WPMFileName.c_str(), fstream::out);
    candFout.open(candFileName.c_str(), fstream::out);
    disFout << "id,scale,size,word,algorithm,sampleNum,coor,distance" << endl;

}

void initDTW()
{
    rep(i, MAXSAMPLE)
        rep(j, MAXSAMPLE)
            dtw[i][j] = inf;
    dtw[0][0] = 0;
}

void initLexicon()
{
    fstream fin;
    fin.open("corpus.txt", fstream::in);
    string s;
    rep(i, LEXICON_SIZE)
        fin >> dict[i] >> freq[i];
    fin.close();
}

vector<Vector2> wordToPath(string word, int id)
{
    vector<Vector2> pts;
    int preKey = -1;
    rep(i, word.length())
    {
        int key = word[i] - 'a';
        if (key != preKey)
            pts.push_back(Vector2(keyPos[key].x * width[id], keyPos[key].y * height[id]));
        preKey = key;
    }
    return pts;
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

    WPM[id] = alpha / (lastT - startTime) * 12;
    fin.close();
}

void outputWPM()
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

void calcDistance(int id, vector<int>& sampleNums)
{
    fstream& fout = disFout;
    int line = 0;
    double keyWidth = width[id] / 10;
    rep(w, words.size())
    {
        string word = words[w];
        vector<Vector2> rawstroke;
        while (line < cmd.size())
        {
            string s = cmd[line];
            Vector2 p(relative[line].x * width[id], relative[line].y * height[id]);
            line++;
            if (rawstroke.size() == 0 || dist(rawstroke[rawstroke.size()-1], p) > eps)
                rawstroke.push_back(p);
            if (s == "Ended")
                break;
        }
        if (word.length() == 1)
            continue;
        vector<Vector2> pts = wordToPath(word, id);
        if (rawstroke.size() <= 1)
            return;
        rep(i, sampleNums.size())
        {
            int &num = sampleNums[i];
            vector<Vector2> location = temporalSampling(pts, num);
            vector<Vector2> stroke = temporalSampling(rawstroke, num);

            double result = match(stroke, location, dtw, Standard) / num;
            fout<< userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "Standard" << ","
                << num << ","
                << "pixel" << ","
                << result << endl;
            fout<< userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "Standard" << ","
                << num << ","
                << "keyWidth" << ","
                << result / keyWidth << endl;

            result = match(stroke, location, dtw, DTW) / num;
            fout<< userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "DTW" << ","
                << num << ","
                << "pixel" << ","
                << result << endl;
            fout<< userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "DTW" << ","
                << num << ","
                << "keyWidth" << ","
                << result / keyWidth << endl;
        }
    }
}

void calcCandidate(int id, vector<int>& sampleNums)
{
    fstream& fout = candFout;
    int line = 0, p = 0, q = 0;

    if (scale[id] == "1x3")
        p = 1;
    if (same(keyboardSize[id], 0.75))
        q = 1;
    else if (same(keyboardSize[id], 1))
        q = 2;
    cout << endl << id << endl;
    rep(w, words.size())
    {
        string word = words[w];
        cout << word << " ";
        vector<Vector2> rawstroke;
        while (line < cmd.size())
        {
            string s = cmd[line];
            Vector2 p(relative[line].x * width[id], relative[line].y * height[id]);
            line++;
            if (rawstroke.size() == 0 || dist(rawstroke[rawstroke.size()-1], p) > eps)
                rawstroke.push_back(p);
            if (s == "Ended")
                break;
        }
        if (word.length() == 1)
            continue;
        if (rawstroke.size() <= 1)
            return;
        wordCount[p][q]++;
        rep(i, sampleNums.size())
        {
            int& num = sampleNums[i];
            vector<Vector2> pts = wordToPath(word, id);
            vector<Vector2> location = temporalSampling(pts, num);
            vector<Vector2> stroke = temporalSampling(rawstroke, num);

            double result = match(stroke, location, dtw, Standard);
            double resultDTW = match(stroke, location, dtw, DTW);
            int stdRank = 1, dtwRank = 1;

            rep(j, LEXICON_SIZE)
            {
                if (word == dict[j])
                    continue;
                pts = wordToPath(dict[j], id);
                location = temporalSampling(pts, num);

                if (stdRank <= 10 && match(stroke, location, dtw, Standard, result) < result)
                {
                    stdRank++;
                    if (stdRank > 10 && dtwRank > 10)
                        break;
                }
                if (dtwRank <= 10 && match(stroke, location, dtw, DTW, resultDTW) < resultDTW)
                {
                    dtwRank++;
                    if (stdRank > 10 && dtwRank > 10)
                        break;
                }
            }
            if (stdRank <= 10)
                stdCount[p][q][i][stdRank]++;
            if (dtwRank <= 10)
                dtwCount[p][q][i][dtwRank]++;
        }
    }
}

void outputCandidate(vector<int> sampleNums)
{
    fstream& fout = candFout;
    fout << "id,scale,size,algorithm,sampleNum,top1,top2,top3,top4,top5,top6,top7,top8,top9,top10" << endl;
    cout << endl;
    rep(p, 2)
    {
        string scale = (p==0)?"1x1":"1x3";
        rep(q, 3)
        {
            string keyboardSize = "0.5";
            if (q == 1)
                keyboardSize = "0.75";
            else if (q == 2)
                keyboardSize = "1";
            rep(i, sampleNums.size())
            {
                For(j, 10)
                {
                    stdCount[p][q][i][j] += stdCount[p][q][i][j-1];
                    dtwCount[p][q][i][j] += dtwCount[p][q][i][j-1];
                }

                fout<< userID << ","
                    << scale << ","
                    << keyboardSize << ","
                    << "standard" << ","
                    << sampleNums[i];
                For(j, 10)
                    fout << "," << stdCount[p][q][i][j] / wordCount[p][q];
                fout << endl;
                fout<< userID << ","
                    << scale << ","
                    << keyboardSize << ","
                    << "DTW" << ","
                    << sampleNums[i];
                For(j, 10)
                    fout << "," << dtwCount[p][q][i][j] / wordCount[p][q];
                fout << endl;
            }
        }
    }
    fout.close();
}

int main()
{
    initFstream("yzc", "1");
    initDTW();
    initLexicon();
    calcKeyLayout();

    int sampleNums[] = {16, 32, 64, 128, 256};
    int candSamples[] = {16, 32, 64};
    vector<int> sample(sampleNums, sampleNums + 5);
    vector<int> candSample(candSamples, candSamples + 3);

    rep(i, PHRASES)
    {
        readData(i);
        calcDistance(i, sample);
        calcCandidate(i, candSample);
    }
    outputWPM();
    outputCandidate(candSample);
    return 0;
}

