#ifndef DATAREADER_H_
#define DATAREADER_H_

#include "Common.h"
#include "Vector2.h"

#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

string sentence[PHRASES], inputText[PHRASES], mode[PHRASES], candMethod[PHRASES], scale[PHRASES];
vector<string> cmd, para, words;
vector<int> paraCnt;
vector<double> time;
vector<Vector2> world, relative;
double height[PHRASES], width[PHRASES], heightRatio[PHRASES], widthRatio[PHRASES], keyboardSize[PHRASES];

void linePushBack(string s, double t, string p, int pc = 0)
{
    cmd.push_back(s);
    time.push_back(t);
    para.push_back(p);
    paraCnt.push_back(pc);
    world.push_back(Vector2(0, 0));
    relative.push_back(Vector2(0, 0));
}

void linePushBack(string s, double t, double x = 0, double y = 0, double rx = 0, double ry = 0)
{
    cmd.push_back(s);
    time.push_back(t);
    para.push_back("");
    paraCnt.push_back(0);
    world.push_back(Vector2(x, y));
    relative.push_back(Vector2(rx, ry));
}

void cleanAll()
{
    cmd.clear(); time.clear();
    para.clear(); paraCnt.clear();
    world.clear(); relative.clear();
}

void sentenceToWords(string sentence, vector<string>& words)
{
    words.clear();
    string word = "";
    rep(i, sentence.length())
        if (sentence[i] >= 'a' && sentence[i] <= 'z')
            word += sentence[i];
        else
        {
            words.push_back(word);
            word = "";
        }
    if (word.length() > 0)
        words.push_back(word);
}

void readData(string fileName, int id)
{
    cout << fileName << endl;
    fstream fin;
    fin.open(fileName.c_str(), fstream::in);
    getline(fin, sentence[id]);
    getline(fin, inputText[id]);
    cout << sentence[id] << endl;
    if (sentence[id] != inputText[id])
    {
        cout << fileName << ": diff" << endl;
        cout << inputText[id] << endl;
    }
    transform(sentence[id].begin(), sentence[id].end(), sentence[id].begin(), ::tolower);
    transform(inputText[id].begin(), inputText[id].end(), inputText[id].begin(), ::tolower);
    fin >> mode[id] >> candMethod[id];
    fin >> widthRatio[id] >> heightRatio[id];
    fin >> width[id] >> height[id];
    if (width[id] > 2 * height[id])
        scale[id] = "1x1";
    else
        scale[id] = "1x3";

    keyboardSize[id] = (widthRatio[id] / 0.8) + 0.25f;

    sentenceToWords(sentence[id], words);

    double startTime = -1;

    cleanAll();

    int n;
    string s, pa;
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
            cleanAll();
            continue;
        }
        lastT = t;
        if (s == "Candidates" || s == "Accept")
        {
            fin >> n;
            getline(fin, pa);
            if (pa.length() > 0)
                pa = pa.substr(1, pa.length() - 1);
            linePushBack(s, t, pa, n);
            continue;
        }
        if (s == "Delete")
        {
            fin >> pa;
            linePushBack(s, t, pa);
            continue;
        }
        if (s == "Cancel" || s == "Expand" || s == "NextCandidatePanel")
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
    fin.close();
}

enum Type
{
    Gesture = 0,
    Select = 1,
    Cancel = 2,
    Delete = 3,
    Prepare = 4,
    TYPENUM = 5,
};

struct TimeSpan
{
    Type type;
    double startTime, endTime;
    int n;
    string para;
};

string typeToString(int type)
{
    switch(type)
    {
        case Gesture: return "Gesture";
        case Select: return "Select";
        case Cancel: return "Cancel";
        case Delete: return "Delete";
        case Prepare: return "Prepare";
    }
}

vector<TimeSpan> span;

void calcListTimeSpan()
{
    bool inRest = false;
    span.clear();
    TimeSpan cnt;
    cnt.startTime = cnt.endTime = -1;
    rep(i, cmd.size())
    {
        cnt.para = para[i];
        cnt.n = paraCnt[i];
        if (cmd[i] == "Candidates")
        {
            if (paraCnt[i] == 0)
                continue;
            cnt.type = Gesture;
            span.push_back(cnt);
            inRest = true;
            cnt.startTime = cnt.endTime;
        }
        else if (cmd[i] == "Delete")
        {
            cnt.type = Delete;
            if (para[i] == "LeftSwipe")
            {
                cnt.startTime = span.back().startTime;
                span.pop_back(); //Prepare
                span.push_back(cnt);
            }
            else cout << "Unknown para for Delete: " << para[i] << i << endl;
            inRest = true;
            cnt.startTime = cnt.endTime;
        }
        else if (cmd[i] == "Accept")
        {
            if (cmd[i+1] == "Candidates")
                continue;
            cnt.type = Select;
            if (span.back().type == Prepare)
            {
                cnt.startTime = span.back().startTime;
                span.pop_back();
            }

            span.push_back(cnt);
            inRest = true;
            cnt.startTime = cnt.endTime;
        }
        else if (cmd[i] == "Cancel")
        {
            cnt.type = Cancel;
            if (span.back().type == Prepare)
            {
                cnt.startTime = span.back().startTime;
                span.pop_back();
            }
            span.push_back(cnt);
            inRest = true;
            cnt.startTime = cnt.endTime;
        }
        else if (cmd[i] == "Began")
        {
            if (inRest)
            {
                cnt.endTime = time[i];
                cnt.type = Prepare;
                span.push_back(cnt);
                inRest = false;
                cnt.startTime = time[i];
            }
            if (cnt.startTime == -1)
                cnt.startTime = time[i];
        }
        else if (cmd[i] == "Stationary" || cmd[i] == "Moved" || cmd[i] == "Ended" || cmd[i] == "Expand")
            cnt.endTime = time[i];
        else if (cmd[i] == "PhraseEnd")
        {

        }
        else
            cout << "Unknown cmd: " << cmd[i] << endl;
    }
}

void calcRadialTimeSpan()
{
    bool inRest = false;
    span.clear();
    TimeSpan cnt;
    cnt.startTime = cnt.endTime = -1;
    rep(i, cmd.size())
    {
        cnt.para = para[i];
        cnt.n = paraCnt[i];
        if (cmd[i] == "Candidates")
        {
            if (paraCnt[i] == 0)
                continue;
            cnt.type = Gesture;
            span.push_back(cnt);
            inRest = false;
            cnt.startTime = cnt.endTime;
        }
        else if (cmd[i] == "Delete")
        {
            cnt.type = Delete;
            if (para[i] == "LeftSwipe")
            {
                cnt.startTime = span.back().startTime;
                span.pop_back(); //Prepare
                span.push_back(cnt);
            }
            else cout << "Unknown para for Delete: " << para[i] << i << endl;
            inRest = true;
            cnt.startTime = cnt.endTime;
        }
        else if (cmd[i] == "Accept")
        {
            cnt.type = Select;
            span.push_back(cnt);
            inRest = true;
            cnt.startTime = cnt.endTime;
        }
        else if (cmd[i] == "Cancel")
        {
            cnt.type = Cancel;
            span.push_back(cnt);
            inRest = true;
            cnt.startTime = cnt.endTime;
        }
        else if (cmd[i] == "Began")
        {
            if (inRest)
            {
                cnt.endTime = time[i];
                cnt.type = Prepare;
                span.push_back(cnt);
                inRest = false;
                cnt.startTime = time[i];
            }
            if (cnt.startTime == -1)
                cnt.startTime = time[i];
        }
        else if (cmd[i] == "Stationary" || cmd[i] == "Moved" || cmd[i] == "Ended")
            cnt.endTime = time[i];
        else if (cmd[i] == "NextCandidatePanel" || cmd[i] == "PhraseEnd")
        {

        }
        else
            cout << "Unknown cmd: " << cmd[i] << endl;
    }
}

void calcTimeSpan(int id)
{
    if (candMethod[id] == "List")
        calcListTimeSpan();
    else
        calcRadialTimeSpan();
    if (span.back().type == Prepare)
        span.pop_back();
}


#endif // DATAREADER_H_
