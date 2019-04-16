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

vector<string> cmd;
vector<string> words;

vector<double> time;
vector<Vector2> world, relative;

int userID;
string name, keyFileName, WPMFileName, disFileName, wordTimeFileName;
fstream keyFout, WPMFout, disFout, wordTimeFout;

void initFstream()
{
    wordTimeFileName = "res/Word_Time_Study2 (Indirect).csv";

    wordTimeFout.open(wordTimeFileName.c_str(), fstream::out);
    wordTimeFout << "id,scale,size,word,time,time/length" << endl;
}

void init()
{
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
                    fout << userID + 1 << "," << scale[id] << "," << keyboardSize[id] << ","
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

    rep(p, USER_NUM)
    {
        name = user[p];
        userID = p;
        cout << userID + 1 << " " << name << endl;
        rep(i, PHRASES)
        {
            readData(i);
            calcWordTime(i);
        }
        //clean();
    }
    return 0;
}

