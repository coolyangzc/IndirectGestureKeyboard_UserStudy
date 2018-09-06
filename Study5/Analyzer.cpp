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
const int RANK = 13;

int top[RANK + 1], topClean[RANK + 1];
int timeNum[TYPENUM], timeNumA[TYPENUM];
double timeCount[TYPENUM], timeBlock[TYPENUM];
double timeBlockA[TYPENUM];

fstream timeFout, tMeanFout, tMeanAllFout, tRatioFout, WPMFout, candFout, candCleanFout, heatFout, speedFout, speedMFout;

void init()
{
    WPMFout.open("res/WPM_Study3.csv", fstream::out);
    candFout.open("res/Candidates_Study3.csv", fstream::out);
    candCleanFout.open("res/Candidates_Clean_Study3.csv", fstream::out);
    timeFout.open("res/Time_Study3.csv", fstream::out);
    tMeanFout.open("res/Time_Mean_Study3.csv", fstream::out);
    tMeanAllFout.open("res/Time_Mean_Sum_Study3.csv", fstream::out);
    tRatioFout.open("res/Time_Ratio_Study3.csv", fstream::out);
    speedFout.open("res/Speed_Study3.csv", fstream::out);
    speedMFout.open("res/Speed_Mean_Study3.csv", fstream::out);

    WPMFout << "id,mode,cand,block,phrase,WPM,N(words),correct,uncorrected,cancel,uncorrectCancel,delete,noexpand" << endl;
    candFout << "id,mode,cand,block,phrase";
    candCleanFout << "id,mode,cand,block,phrase";
    timeFout << "id,mode,cand,block,phrase";
    heatFout << "id,mode,cand,block,phrase";
    tMeanFout << "id,mode,cand,block,phrase";
    tMeanAllFout << "id,mode,cand";
    tRatioFout << "id,mode,cand,block,phrase";
    speedMFout << "id,mode,cand,block" << ",part(#/50),GestureSpeed(keyW/s)" << endl;
    speedFout << "id,mode,cand,block,phrase";
    For(i, 50)
        speedFout << "," << i;
    speedFout << endl;
    rep(i, TYPENUM)
    {
        timeFout << "," << typeToString(i) << "," << typeToString(i) << "(N)";
        tMeanFout << "," << typeToString(i) << "(Mean)";
        tMeanAllFout << "," << typeToString(i) << "(Mean)";
        tRatioFout << "," << typeToString(i) << "(Ratio)";
    }
    timeFout << endl;
    tMeanFout << endl;
    tMeanAllFout << endl;
    tRatioFout << endl;
    candFout << ",Top1,Top2,Top3,Top4,Top5,Top6,Top7,Top8,Top9,Top10,Top11,Top12,Top13,All"
             << ",Top1(ratio),Top2(ratio),Top3(ratio),Top4(ratio),Top5(ratio),Top6(ratio),Top7(ratio)"
             << ",Top8(ratio),Top9(ratio),Top10(ratio),Top11(ratio),Top12(ratio),Top13(ratio)" << endl;
    candCleanFout << ",Top1,Top2,Top3,Top4,Top5,Top6,Top7,Top8,Top9,Top10,Top11,Top12,Top13,All"
                  << ",Top1(ratio),Top2(ratio),Top3(ratio),Top4(ratio),Top5(ratio),Top6(ratio),Top7(ratio)"
                  << ",Top8(ratio),Top9(ratio),Top10(ratio),Top11(ratio),Top12(ratio),Top13(ratio)" << endl;
}

void outputBasicInfo(fstream& fout, int user, int id)
{
    fout << user << ","
         << mode[id] << ","
         << candMethod[id] << ","
         << (id / 10) % 4 + 1 << ","
         << id % 40 + 1;
}


void calcTimeDistribution(int user, int id)
{
    outputBasicInfo(timeFout, user, id);
    int timeN[TYPENUM];
    memset(timeN, 0, sizeof(timeN));
    memset(timeCount, 0, sizeof(timeCount));
    rep(i, span.size())
    {
        timeN[span[i].type]++;
        timeNum[span[i].type]++;
        timeCount[span[i].type] += span[i].endTime - span[i].startTime;
        timeBlock[span[i].type] += span[i].endTime - span[i].startTime;

        timeNumA[span[i].type]++;
        timeBlockA[span[i].type] += span[i].endTime - span[i].startTime;
    }
    rep(i, TYPENUM)
        timeFout << "," << timeCount[i] << "," << timeN[i];

    timeFout << endl;

    if ((id + 1) % 10 == 0)
    {
        outputBasicInfo(tMeanFout, user, id);
        outputBasicInfo(tRatioFout, user, id);
        double tot = 0;
        rep(i, TYPENUM)
            tot += timeBlock[i];
        rep(i, TYPENUM)
        {
            tMeanFout << "," << timeBlock[i] / timeNum[i];
            tRatioFout << "," << timeBlock[i] / tot;
        }
        tMeanFout << endl;
        tRatioFout << endl;
        memset(timeNum, 0, sizeof(timeNum));
        memset(timeBlock, 0, sizeof(timeBlock));
    }
    if ((id + 1) % 40 == 0)
    {
        tMeanAllFout << user << ","
                     << mode[id] << ","
                     << candMethod[id];
        rep(i, TYPENUM)
            tMeanAllFout << "," << timeBlockA[i] / timeNumA[i];
        tMeanAllFout << endl;
        memset(timeNumA, 0, sizeof(timeNumA));
        memset(timeBlockA, 0, sizeof(timeBlockA));
    }
}

void printTimeSpan()
{
    rep(i, span.size())
    {
        cout << "  " << typeToString(span[i].type) << ": ";
        cout << span[i].startLine << " - ";
        cout << span[i].endLine << "; ";
        cout << span[i].startTime << " - ";
        cout << span[i].endTime << "; ";
        cout << span[i].para << "; ";
        cout << span[i].n << endl;
    }

}

void calcWPM(int user, int id)
{
    int deleteCnt = 0, cancelCnt = 0, uncorrectCancel = 0, correct = 0, uncorrected = 0, noexpand = 0;
    vector<string> candidates;
    int wordP = -1;
    bool same = false;
    double phraseEndTime = -1;
    rep(i, span.size())
    {
        if (span[i].type == Delete)
        {
            deleteCnt++, wordP = max(wordP - 1, -1);

            if (candMethod[id] == "List")
            {
                if (same)
                {
                    uncorrectCancel++;
                    same = false;
                }
                if (i && span[i-1].type == Gesture)
                    noexpand++;
            }

        }
        else if (span[i].type == Cancel)
        {
            cancelCnt++;
            if (same)
            {
                uncorrectCancel++;
                same = false;
            }

            if (candMethod[id] == "Radial")
            {
                wordP--;
                bool expand = false;
                FOR(k, span[i].startLine + 1, span[i].endLine)
                    if (cmd[k] == "NextCandidatePanel")
                    {
                        expand = true;
                        break;
                    }
                if (!expand) noexpand++;
            }

        }
        else if (span[i].type == Gesture)
        {
            sentenceToWords(span[i].para, candidates);
            wordP++;
            if (wordP + 1 == words.size())
                phraseEndTime = span[i].endTime;
            same = false;
            rep(i, candidates.size())
            {
                if (candidates[i] == words[wordP])
                {
                    same = true;
                    break;
                }
            }
        }
        else if (span[i].type == Select)
        {
            if (wordP + 1 == words.size())
                phraseEndTime = span[i].endTime;
        }
    }
    vector<string> inputWords;
    sentenceToWords(inputText[id], inputWords);
    rep(i, words.size())
        if (i >= inputWords.size() || words[i] != inputWords[i])
            uncorrected++;
        else
            correct++;
    if (inputWords.size() > words.size())
        uncorrected += inputWords.size() - words.size();


    if (phraseEndTime == -1)
    {
        while(1);
    }
    double totTime = span.back().endTime - span.front().startTime;
    if (mode[id] == "FixStart")
        totTime = phraseEndTime - span.front().startTime;

    double wpm = inputText[id].length() / totTime * 12;
    outputBasicInfo(WPMFout, user, id);
    WPMFout << ","
            << wpm << ","
            << words.size() << ","
            << correct << "," << uncorrected << ","
            << cancelCnt << "," << uncorrectCancel << "," << deleteCnt
            << "," << noexpand
            << endl;
}
void calcCandidate(int user, int id)
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
            {
                top[RANK]++;
                continue;
            }

            int same = -1;
            rep(j, candidates.size())
            {
                if (candidates[j] == words[wordP])
                {
                    top[same = j]++; topClean[j]++;
                    break;
                }
            }
            if (same < 0)
            {
                //Length Check for Clean
                top[RANK]++;
                vector<Vector2> p; p.clear();
                FOR(k, span[i].startLine + 1, span[i].endLine)
                    if (cmd[k] == "Began" || cmd[k] == "Moved" || cmd[k] == "Stationary" || cmd[k] == "Ended")
                        p.push_back(Vector2(relative[k].x, relative[k].y * 0.3 * 3));
                double len = 0, lenWord = 0;
                if (p.size() > 1)
                    rep(k, p.size() - 1)
                        len += dist(p[k], p[k+1]);
                string word = (mode[id] == "FixStart")?"g":"" + words[wordP];

                vector<Vector2> v = wordToPath(word, 3);
                rep(k, v.size() - 1)
                    lenWord += dist(v[k], v[k+1]);
                if (len < lenWord * 3 + 0.1 && lenWord < len * 3 + 0.1)
                    topClean[RANK]++;
            }
        }
    }

    if ((id+1) % 10 == 0)
    {
        For(i, RANK)
        {
            top[i] += top[i-1];
            topClean[i] += topClean[i-1];
        }

        outputBasicInfo(candFout, user, id);
        outputBasicInfo(candCleanFout, user, id);
        rep(i, RANK + 1)
        {
            candFout << "," << top[i];
            candCleanFout << "," << topClean[i];
        }
        rep(i, RANK)
        {
            candFout << "," << (float)top[i] / top[RANK];
            candCleanFout << "," << (float)topClean[i] / topClean[RANK];
        }
        candFout << endl;
        candCleanFout << endl;
        memset(top, 0, sizeof(top));
        memset(topClean, 0, sizeof(topClean));
    }

}

int speedCnt;
double speed[100 + 1];

void calcSpeed(int user, int id)
{
    vector<Vector2> p;
    vector<double> t;
    double keyW = width[id] / 10;
    rep(s, span.size())
        if (span[s].type == Gesture)
        {
            p.clear(); t.clear();
            FOR(i, span[s].startLine + 1, span[s].endLine)
                if (cmd[i] == "Began" || cmd[i] == "Moved" || cmd[i] == "Stationary" || cmd[i] == "Ended")
                {
                    p.push_back(world[i]);
                    t.push_back(time[i]);
                }
            if (p.empty())
                continue;
            double len = 0;
            rep(i, p.size() - 1)
                len += dist(p[i], p[i+1]);

            if (len < eps)
                continue;
            outputBasicInfo(speedFout, user, id);
            len /= 50;
            int part = 1;
            speedCnt++;

            double period = 0, g = 0, a = 0;
            rep(i, p.size() - 1)
            {
                double d = dist(p[i+1], p[i]), dtime = t[i+1] - t[i];
                while (g + d > len)
                {
                    a = (len - g) / d;
                    period += dtime * a;
                    speed[part] += len / keyW / period;
                    part++;
                    speedFout << "," << len / keyW / period;
                    d -= len - g;
                    dtime *= 1 - a;
                    g = period = 0;
                }
                g += d;
                period += dtime;
            }
            if (part <= 50)
            {
                speed[part] += len / keyW / period;
                speedFout << "," << len / keyW / period;
            }
            speedFout << endl;
        }
    if ((id + 1)% 10 == 0)
    {
        For(i, 50)
        {
            speedMFout << user << ","
                       << mode[id] << ","
                       << candMethod[id] << ","
                       << (id / 10) % 4 + 1 << ","
                       << i << ","
                       << speed[i] / speedCnt << endl;
        }
        speedCnt = 0;
        memset(speed, 0, sizeof(speed));
    }
}

void calcHeat(int user, int id)
{

}

stringstream ss;

int main()
{
    init();
    calcKeyLayout();
    FOR(p, USER_L, USER_NUM)
    //FOR(p, 16, 16)
    {
        rep(i, 80)
        {
            ss.clear();ss.str("");
            ss << i;
            string fileName = "data/" + user[p] + "_" + ss.str() + ".txt";
            readData(fileName, i);
            calcTimeSpan(i);
            //printTimeSpan();
            calcTimeDistribution(p, i);
            calcWPM(p, i);
            calcCandidate(p, i);
            calcSpeed(p, i);
            calcHeat(p, i);
        }
    }
    WPMFout.close();
    candFout.close();
    timeFout.close();
    speedFout.close();
    tMeanFout.close();
    tMeanAllFout.close();
    speedMFout.close();
    tRatioFout.close();
    return 0;
}

