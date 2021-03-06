#include "Keyboard.h"

#include <cstdio>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

const int SAMPLE_NUM = 32;

string sentence[PHRASES], mode[PHRASES], scale[PHRASES];
double height[PHRASES], width[PHRASES], heightRatio[PHRASES], widthRatio[PHRASES], keyboardSize[PHRASES];
int wordCnt[PHRASES];
double WPM[PHRASES];

double dtw[MAXSAMPLE][MAXSAMPLE];

vector<string> cmd;
vector<string> words;

vector<double> time;
vector<Vector2> world, relative;

string name, userID, keyFileName, WPMFileName, disFileName, wordTimeFileName;
fstream keyFout, WPMFout, disFout, wordTimeFout;

void initFstream()
{
    keyFileName = "res/Key_Study1.csv";
    WPMFileName = "res/WPM_Study1.csv";
    disFileName = "res/Distance_Study1.csv";
    wordTimeFileName = "res/Word_Time_Study1.csv";

    keyFout.open(keyFileName.c_str(), fstream::out);
    keyFout << "id,scale,size,word,kind,keyWidth" << endl;
    WPMFout.open(WPMFileName.c_str(), fstream::out);
    WPMFout << "id,scale,size,sentence,WPM,words" << endl;
    disFout.open(disFileName.c_str(), fstream::out);
    disFout << "id,scale,size,word,algorithm,distance" << endl;
    wordTimeFout.open(wordTimeFileName.c_str(), fstream::out);
    wordTimeFout << "id,scale,size,word,time,time/length" << endl;
}

void init()
{
    initKeyboard(dtw);
    initFstream();
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

    keyboardSize[id] = (widthRatio[id] / 0.8) + 0.25f;

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
            //linePushBack(s, t);
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
    wordCnt[id] = words.size();
    fin.close();

}

void outputWPM()
{
    rep(i, PHRASES)
    {
        WPMFout << userID << ","
                << scale[i] << ","
                << keyboardSize[i] << ","
                << sentence[i] << ","
                << WPM[i] << ","
                << wordCnt[i] << endl;
    }
}

void calcDistance(int id)
{
    int line = 0;
    double keyWidth = width[id] / 10;

    rep(w, words.size())
    {
        string word = words[w];
        vector<Vector2> rawstroke;
        while (line < cmd.size())
        {
            Vector2 p(relative[line].x * width[id], relative[line].y * height[id]);
            line++;
            if (rawstroke.size() == 0 || dist(rawstroke[rawstroke.size()-1], p) > eps)
                rawstroke.push_back(p);
            if (cmd[line] == "Ended")
                break;
        }
        if (word.length() == 1)
            continue;
        vector<Vector2> pts = wordToPath(word, id);
        if (rawstroke.size() <= 1)
            return;
        double firstKeyDis = dist(pts[0], rawstroke[0]);
        double lastKeyDis = dist(pts[pts.size() - 1], rawstroke[rawstroke.size() - 1]);
        vector<Vector2> location = temporalSampling(pts, SAMPLE_NUM);
        vector<Vector2> stroke = temporalSampling(rawstroke, SAMPLE_NUM);
        double middleKeyDis = 0;
        FOR(i, 1, SAMPLE_NUM - 2)
            middleKeyDis += dist(location[i], stroke[i]);
        middleKeyDis /= (SAMPLE_NUM - 2);
        keyFout << userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "FirstKey" << ","
                << firstKeyDis / keyWidth << endl;
        keyFout << userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "MiddleKey" << ","
                << middleKeyDis / keyWidth << endl;
        keyFout << userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "LastKey" << ","
                << lastKeyDis / keyWidth << endl;
        disFout << userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "SHARK2" << ","
                << match(location, stroke, dtw, Standard) / keyWidth << endl;
        disFout << userID << ","
                << scale[id] << ","
                << keyboardSize[id] << ","
                << word << ","
                << "DTW" << ","
                << match(location, stroke, dtw, DTW) / keyWidth << endl;
    }
}

void calcWordTime(int id)
{
    fstream& fout = wordTimeFout;
    int line = 0;
    rep(w, words.size())
    {
        double s = -1, t = -1;
        while (line < cmd.size())
        {
            if (cmd[line] == "Began")
                s = time[line];
            line++;
            if (cmd[line] == "Ended")
            {
                t = time[line];
                if (s != -1)
                    fout << userID << "," << scale[id] << "," << keyboardSize[id] << ","
                         << words[w] << ","
                         << t - s << "," << (t - s) / words[w].length() << endl;
                break;
            }
        }
    }
}

int main()
{
    init();

    int candSamples[] = {32};
    vector<int> candSample(candSamples, candSamples + 1);
    rep(p, USER_NUM)
    {
        name = user[p];
        userID = id[p];
        cout << userID << " " << name << endl;
        rep(i, PHRASES)
        {
            readData(i);
            calcDistance(i);
            calcWordTime(i);
        }
        outputWPM();
        //clean();
    }
    return 0;
}

