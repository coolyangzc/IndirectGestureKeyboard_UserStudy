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

int top[RANK + 1];
int timeNum[TYPENUM];
double timeCount[TYPENUM], timeBlock[TYPENUM];

fstream timeFout, tMeanFout, tRatioFout, WPMFout, candFout;

void init()
{
    WPMFout.open("res/WPM_Study3.csv", fstream::out);
    candFout.open("res/Candidates_Study3.csv", fstream::out);
    timeFout.open("res/Time_Study3.csv", fstream::out);
    tMeanFout.open("res/Time_Mean_Study3.csv", fstream::out);
    tRatioFout.open("res/Time_Ratio_Study3.csv", fstream::out);
    WPMFout << "id,mode,cand,block,phrase,WPM,N(words),correct,uncorrected,cancel,uncorrectCancel,delete" << endl;
    candFout << "id,mode,cand,block,phrase";
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
    candFout << ",Top1,Top2,Top3,Top4,Top5,Top6,Top7,Top8,Top9,Top10,Top11,Top12,Top13,All"
             << ",Top1(ratio),Top2(ratio),Top3(ratio),Top4(ratio),Top5(ratio),Top6(ratio),Top7(ratio)"
             << ",Top8(ratio),Top9(ratio),Top10(ratio),Top11(ratio),Top12(ratio),Top13(ratio)" << endl;
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

void printTimeSpan()
{
    rep(i, span.size())
    {
        cout << "  " << typeToString(span[i].type) << ": ";
        cout << span[i].startTime << " - ";
        cout << span[i].endTime << "; ";
        cout << span[i].para << "; ";
        cout << span[i].n << endl;
    }

}

void calcWPM(int user, int id)
{
    int deleteCnt = 0, cancelCnt = 0, uncorrectCancel = 0, correct = 0, uncorrected = 0;
    vector<string> candidates;
    int wordP = -1;
    bool same = false;
    double phraseEndTime = -1;
    rep(i, span.size())
    {
        if (span[i].type == Delete)
        {
            deleteCnt++, wordP = max(wordP - 1, -1);

            if (candMethod[id] == "List" && same)
                uncorrectCancel++;
        }
        else if (span[i].type == Cancel)
        {
            cancelCnt++;
            same = false;
            rep(i, candidates.size())
            {
                if (candidates[i] == words[wordP])
                {
                    same = true;
                    break;
                }
            }
            if (same)
                uncorrectCancel++;
            if (candMethod[id] == "Radial")
                wordP--;
        }
        else if (span[i].type == Gesture)
        {
            sentenceToWords(span[i].para, candidates);
            wordP++;
            if (wordP + 1 == words.size())
                phraseEndTime = span[i].endTime;
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


    //double totTime = span.back().endTime - span.front().startTime;
    double totTime = phraseEndTime - span.front().startTime;
    double wpm = inputText[id].length() / totTime * 12;
    outputBasicInfo(WPMFout, user, id);
    WPMFout << ","
            << wpm << ","
            << words.size() << ","
            << correct << "," << uncorrected << ","
            << cancelCnt << "," << uncorrectCancel << "," << deleteCnt
            << endl;
}

void calcCandidate(int user, int id)
{
    vector<int> tops;
    tops.clear();
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
            int same = -1;
            rep(j, candidates.size())
            {
                if (candidates[j] == words[wordP])
                {
                    top[same = j]++;
                    break;
                }
            }
            if (same < 0)
                top[RANK]++;
        }
    }

    if ((id+1) % 10 == 0)
    {
        For(i, RANK)
            top[i] += top[i-1];
        outputBasicInfo(candFout, user, id);
        rep(i, RANK + 1)
            candFout << "," << top[i];
        rep(i, RANK)
            candFout << "," << (float)top[i] / top[RANK];
        candFout << endl;
        memset(top, 0, sizeof(top));
    }

}

stringstream ss;

int main()
{
    init();
    FOR(p, USER_L - 1, USER_NUM - 1)
    //FOR(p, 12, 12)
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
        }
    }
    WPMFout.close();
    candFout.close();
    timeFout.close();
    tMeanFout.close();
    tRatioFout.close();
    return 0;
}

