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

int timeNum[TYPENUM];
double timeCount[TYPENUM], timeBlock[TYPENUM];

fstream timeFout, tMeanFout, tRatioFout, WPMFout;

void init()
{
    WPMFout.open("res/WPM_Study3.csv", fstream::out);
    timeFout.open("res/Time_Study3.csv", fstream::out);
    tMeanFout.open("res/Time_Mean_Study3.csv", fstream::out);
    tRatioFout.open("res/Time_Ratio_Study3.csv", fstream::out);
    WPMFout << "id,mode,cand,block,phrase,WPM,N(words),correct,uncorrected,cancel,uncorrectCancel,delete" << endl;
    timeFout << "id,mode,cand,block,phrase";
    tMeanFout << "id,mode,cand,block,phrase";
    tRatioFout << "id,mode,cand,block,phrase";
    rep(i, TYPENUM)
    {
        timeFout << "," << typeToString(i);
        tMeanFout << "," << typeToString(i) << "(Mean)";
        tRatioFout << "," << typeToString(i) << "(Ratio)";
    }
    timeFout << endl;
    tMeanFout << endl;
    tRatioFout << endl;
}

void clean()
{

}

void outputBasicInfo(fstream& fout, int user, int id)
{
    fout << user + 1 << ","
         //<< qwertyRating[user] << ","
         //<< gestureKeyboardRating[user] << ","
         << mode[id] << ","
         << candMethod[id] << ","
         << (id / 10) % 4 + 1 << ","
         << id % 40 + 1;
}


void calcTimeDistribution(int user, int id)
{
    outputBasicInfo(timeFout, user, id);

    memset(timeCount, 0, sizeof(timeCount));
    rep(i, span.size())
    {
        cout << "  " << typeToString(span[i].type) << ": ";
        cout << span[i].startTime << " - ";
        cout << span[i].endTime << endl;
        timeNum[span[i].type]++;
        timeCount[span[i].type] += span[i].endTime - span[i].startTime;
        timeBlock[span[i].type] += span[i].endTime - span[i].startTime;
    }
    rep(i, TYPENUM)
        timeFout << "," << timeCount[i];
    timeFout << endl;

    if ((id +1) % 10 == 0)
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
}

void calcWPM(int user, int id)
{
    int deleteCnt = 0, cancelCnt = 0, uncorrectCancel = 0, correct = 0, uncorrected = 0;
    vector<string> candidates;
    int wordP = -1;
    rep(i, span.size())
    {
        if (span[i].type == Delete)
            deleteCnt++, wordP--;
        else if (span[i].type == Cancel)
        {
            cancelCnt++;
            bool same = false;
            rep(i, candidates.size())
            {
                cout << candidates[i] << ",";
                if (candidates[i] == words[wordP])
                {
                    same = true;
                    break;
                }
            }
            cout << endl;
            if (same)
                uncorrectCancel++;
            if (candMethod[id] == "Radial")
                wordP--;
        }

        if (span[i].type == Gesture)
        {
            sentenceToWords(span[i].para, candidates);
            wordP++;
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
    double totTime = span.back().endTime - span.front().startTime;
    double wpm = inputText[id].length() / totTime * 12;
    outputBasicInfo(WPMFout, user, id);
    WPMFout << ","
            << wpm << ","
            << words.size() << ","
            << correct << "," << uncorrected << ","
            << cancelCnt << "," << uncorrectCancel << "," << deleteCnt
            << endl;
}

stringstream ss;

int main()
{
    init();
    FOR(p, USER_L - 1, USER_NUM - 1)
    {
        rep(i, 80)
        {
            ss.clear();ss.str("");
            ss << i;
            string fileName = "data/" + user[p] + "_" + ss.str() + ".txt";
            readData(fileName, i);
            calcTimeSpan(i);
            calcTimeDistribution(p, i);
            calcWPM(p, i);
        }
    }
    WPMFout.close();
    timeFout.close();
    tMeanFout.close();
    tRatioFout.close();
    return 0;
}

