#include "Common.h"
#include "Vector2.h"

#include <cstdio>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

const int USER_NUM = 6;
const string user[USER_NUM] = {"yzc", "maye", "ssy", "xwj", "yyk", "yzp"};
const string id[USER_NUM] = {"1", "2", "3", "4", "5", "6"};

double dtw[MAXSAMPLE][MAXSAMPLE];

string sentence[PHRASES], userText[PHRASES], mode[PHRASES], scale[PHRASES];
double height[PHRASES], width[PHRASES], heightRatio[PHRASES], widthRatio[PHRASES], keyboardSize[PHRASES];
double WPM[PHRASES], totTime[PHRASES];
Vector2 keyPos[26];

double gestureTime[2], selectTime[2], cancelTime[2], deleteTime[2], restTime[2];

string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];
vector<Vector2> path[LEXICON_SIZE];

vector<string> cmd;
vector<string> words;

vector<double> time;
vector<Vector2> world, relative;

string name, userID, timeFileName, WPMFileName;
fstream timeFout, WPMFout;

void initFstream()
{
    WPMFileName = "res/WPM.csv";
    timeFileName = "res/Time.csv";
    WPMFout.open(WPMFileName.c_str(), fstream::out);
    timeFout.open(timeFileName.c_str(), fstream::out);
    WPMFout << "id,order,mode,sentence,WPM" << endl;
    timeFout << "id,mode,kind,time" << endl;
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
    getline(fin, userText[id]);
    if (sentence[id] != userText[id])
        cout << "diff" << endl;
    fin >> mode[id];
    fin >> widthRatio[id] >> heightRatio[id];
    fin >> width[id] >> height[id];
    if (width[id] > 2 * height[id])
        scale[id] = "1x1";
    else
        scale[id] = "1x3";

    keyboardSize[id] = widthRatio[id] / 0.8;

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

    string s, unUsed;
    double t, x, y, rx, ry, lastT;
    while (fin >> s)
    {
        fin >> t;
        if (s == "PhraseEnd")
        {
            linePushBack(s, t);
            break;
        }
        lastT = t;
        if (s == "Candidates")
        {
            int num;
            fin >> num;
            rep(i, num)
                fin >> unUsed;
            linePushBack(s, t);
            continue;
        }
        if (s == "Backspace")
        {
            startTime = -1;
            cmd.clear(); time.clear();
            world.clear(); relative.clear();
            linePushBack(s, t);
            continue;
        }
        if (s == "Accept" || s == "SingleKey")
        {
            fin >> unUsed;
            linePushBack(s, t);
            continue;
        }
        if (s == "Cancel" || s == "Delete")
        {
            linePushBack(s, t);
            continue;
        }
        fin >> x >> y >> rx >> ry;
        linePushBack(s, t, x, y, rx, ry);
        if (s == "Began")
        {
            if (startTime == -1)
                startTime = t;
        }
    }
    totTime[id] = lastT - startTime;
    WPM[id] = alpha / totTime[id] * 12;
    fin.close();
}

void calcTimeDistribution(int id)
{
    fstream& fout = timeFout;
    double startTime = -1, endTime = -1;
    bool inRest = false, inCandidates = false;
    int m = 0;
    if (mode[id] == "FixStart")
        m = 1;
    rep(line, cmd.size())
    {
        string& s = cmd[line];
        if (s == "Candidates")
        {
            cout << "Gesture:" << startTime << " " << endTime << endl;
            gestureTime[m] += endTime - startTime;
            inCandidates = true;
            startTime = time[line];
        }
        else if (s == "Delete")
        {
            cout << "Delete:" << startTime << " " << endTime << endl;
            deleteTime[m] += endTime - startTime;
            inRest = true;
            startTime = endTime;
        }
        else if (s == "SingleKey")
        {
            cout << "SingleKey:" << startTime << " " << endTime << endl;
            gestureTime[m] += endTime - startTime;
            inRest = true;
            startTime = endTime;
        }
        else if (s == "Accept")
        {
            cout << "Accept:" << startTime << " " << endTime << endl;
            selectTime[m] += endTime - startTime;
            inRest = true;
            inCandidates = false;
            startTime = endTime;
        }
        else if (s == "Cancel")
        {
            cout << "Cancel:" << startTime << " " << endTime << endl;
            cancelTime[m] += endTime - startTime;
            inRest = true;
            inCandidates = false;
            startTime = endTime;
        }

        else if (s == "Began")
        {
            if (inRest)
            {
                cout << "Rest:" << startTime << " " << time[line] << endl;
                restTime[m] += time[line] - startTime;
                inRest = false;
                startTime = time[line];
            }
            else
                endTime = time[line];
            if (startTime == -1)
                startTime = time[line];
        }
        else
            endTime = time[line];
    }
}

void outputWPM()
{
    int m = 0;
    double maxWPM[2] = {0, 0};
    rep(i, PHRASES)
    {
        WPMFout << userID << ","
                << (i%10) << ","
                << mode[i] << ","
                << sentence[i] << ","
                << WPM[i] << endl;
        if (mode[i] == "Basic")
            m = 0;
        else
            m = 1;
        maxWPM[m] = max(WPM[i], maxWPM[m]);
    }
    WPMFout << userID << ","
            << "N/A" << ","
            << "max(Basic)" << ","
            << "N/A" << ","
            << maxWPM[0] << endl;
    WPMFout << userID << ","
            << "N/A" << ","
            << "max(FixStart)" << ","
            << "N/A" << ","
            << maxWPM[1] << endl;
}

void outputTimeDistribution()
{
    rep(i, 2)
    {
        double tot = gestureTime[i] + selectTime[i] + cancelTime[i] + deleteTime[i] + restTime[i];
        string mode = ((i==0)?"Basic":"FixStart");
        timeFout<< userID << "," << mode << ","
                << "gesture" << ","
                << gestureTime[i] / tot << endl;
        timeFout<< userID << "," << mode << ","
                << "select" << ","
                << selectTime[i] / tot << endl;
        timeFout<< userID << "," << mode << ","
                << "cancel" << ","
                << cancelTime[i] / tot << endl;
        timeFout<< userID << "," << mode << ","
                << "delete" << ","
                << deleteTime[i] / tot << endl;
        timeFout<< userID << "," << mode << ","
                << "rest" << ","
                << restTime[i] / tot << endl;
    }

}

int main()
{
    initFstream();
    rep(p, USER_NUM)
    {
        name = user[p];
        userID = id[p];
        rep(i, PHRASES)
        {
            readData(i);
            calcTimeDistribution(i);
        }
        outputWPM();
        outputTimeDistribution();
    }
    return 0;
}

