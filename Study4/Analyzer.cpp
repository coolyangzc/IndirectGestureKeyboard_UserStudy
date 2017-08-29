#include "Common.h"
#include "Vector2.h"

#include <cstdio>
#include <vector>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

double dtw[MAXSAMPLE][MAXSAMPLE];

string sentence[PHRASES], userText[PHRASES], mode[PHRASES], scale[PHRASES];
double height[PHRASES], width[PHRASES], heightRatio[PHRASES], widthRatio[PHRASES], keyboardSize[PHRASES];
double WPM[PHRASES], totTime[PHRASES];
int top[13];
Vector2 keyPos[26];

string algorithm;
string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];
vector<Vector2> path[LEXICON_SIZE];

vector<string> cmd, para, words;
vector<int> para0;
vector<double> time;

enum Type
{
    Gesture = 0,
    Select = 1,
    Cancel = 2,
    Delete = 3,
    Rest = 4,
    TYPENUM = 5,
};

double timeCount[TYPENUM], timeBlock[TYPENUM];

string typeToString(int type)
{
    switch(type)
    {
        case Gesture: return "Gesture";
        case Select: return "Select";
        case Cancel: return "Cancel";
        case Delete: return "Delete";
        case Rest: return "Rest";
    }
}
struct TimeSpan
{
    Type type;
    double startTime, endTime;
    int n;
    string para;
};

vector<TimeSpan> span;
vector<Vector2> world, relative;

string name, userID, timeFileName, timeRatioFileName, WPMFileName, candFileName;
fstream timeFout, timeRatioFout, WPMFout, candFout;

void initFstream()
{
    WPMFileName = "res/WPM_Study2.csv";
    timeFileName = "res/Time_Study2.csv";
    timeRatioFileName = "res/Time_Ratio_Study2.csv";
    candFileName = "res/Candidates_Study2.csv";
    WPMFout.open(WPMFileName.c_str(), fstream::out);
    timeFout.open(timeFileName.c_str(), fstream::out);
    timeRatioFout.open(timeRatioFileName.c_str(), fstream::out);
    candFout.open(candFileName.c_str(), fstream::out);

    WPMFout << "id,algorithm,block,mode,sentence,WPM,correct,uncorrected,cancel,delete" << endl;
    timeFout << "id,algorithm,block,mode,sentence";
    timeRatioFout << "id,algorithm,block,mode";
    rep(i, TYPENUM)
    {
        timeFout << "," << typeToString(i);
        timeRatioFout << "," << typeToString(i) << "(ratio)";
    }
    timeFout << endl;
    timeRatioFout << endl;
    candFout << "id,algorithm,block,mode,top1,top2,top3,top4,top5,top6,top7,top8,top9,top10,top11,top12,all,";
    candFout << "top1(ratio),top2(ratio),top3(ratio),top4(ratio),top5(ratio),top6(ratio),"
             << "top7(ratio),top8(ratio),top9(ratio),top10(ratio),top11(ratio),top12(ratio)" << endl;
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

void sentenceToWords(string sentence, vector<string>& words)
{
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

void linePushBack(string s, double t, string p, int p0 = 0)
{
    cmd.push_back(s);
    time.push_back(t);
    para0.push_back(p0);
    para.push_back(p);
    world.push_back(Vector2(0, 0));
    relative.push_back(Vector2(0, 0));
}

void linePushBack(string s, double t, double x = 0, double y = 0, double rx = 0, double ry = 0)
{
    cmd.push_back(s);
    time.push_back(t);
    para0.push_back(0);
    para.push_back("");
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
    cout << id << ": " << sentence[id] << endl;
    if (sentence[id] != userText[id])
    {
        cout << id << ": diff" << endl;
        cout << id << ": " << userText[id] << endl;
    }

    fin >> mode[id];
    fin >> widthRatio[id] >> heightRatio[id];
    fin >> width[id] >> height[id];
    if (width[id] > 2 * height[id])
        scale[id] = "1x1";
    else
        scale[id] = "1x3";

    keyboardSize[id] = (widthRatio[id] / 0.8) + 0.25f;

    words.clear();
    sentenceToWords(sentence[id], words);

    double startTime = -1;
    cmd.clear(); time.clear();
    para.clear(); para0.clear();
    world.clear(); relative.clear();

    int n;
    string s, pa, unUsed;
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
        if (s == "Candidates" || s == "Accept")
        {
            fin >> n;
            getline(fin, pa);
            if (pa.length() > 0)
                pa = pa.substr(1, pa.length() - 1);
            linePushBack(s, t, pa, n);
            continue;
        }
        if (s == "Backspace")
        {
            startTime = -1;
            cmd.clear(); time.clear();
            para.clear(); para0.clear();
            world.clear(); relative.clear();
            continue;
        }
        if (s == "Delete")
        {
            fin >> pa;
            linePushBack(s, t, pa);
            continue;
        }
        if (s == "Cancel" || s == "NextCandidatePanel")
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
    int alpha = userText[id].length();
    WPM[id] = alpha / totTime[id] * 12;
    fin.close();
}

void calcTimeDistribution(int id)
{
    fstream& fout = timeFout;
    double s = -1, t = -1;
    bool inRest = false, inCandidates = false;
    span.clear();
    TimeSpan cnt;
    cnt.startTime = cnt.endTime = -1;
    rep(line, cmd.size())
    {
        cnt.para = para[line];
        cnt.n = para0[line];
        if (cmd[line] == "Candidates")
        {
            if (para0[line] == 0)
                continue;
            cnt.type = Gesture;
            span.push_back(cnt);
            cnt.startTime = cnt.endTime;
            inRest = false;
            inCandidates = true;
        }
        else if (cmd[line] == "Delete")
        {
            cnt.type = Delete;
            if (para[line] == "LeftSwipe")
            {
                cnt.startTime = span.back().startTime;
                span.pop_back(); //Rest
                span.push_back(cnt);
            }
            else if (para[line] == "DoubleClick")
            {
                span.pop_back(); //Cancel
                span.pop_back(); //Gesture
                cnt.startTime = span.back().startTime;
                if (!span.empty())
                    span.pop_back(); //Rest
                span.push_back(cnt);
            }
            else cout << "Unkonwn para for Delete: " << para[line] << line << endl;
            inRest = true;
            inCandidates = false;
            cnt.startTime = cnt.endTime;
        }
        else if (cmd[line] == "Accept")
        {
            cnt.type = Select;
            span.push_back(cnt);
            cnt.startTime = cnt.endTime;
            inRest = true;
            inCandidates = false;
        }
        else if (cmd[line] == "Cancel")
        {
            cnt.type = Cancel;
            span.push_back(cnt);
            cnt.startTime = cnt.endTime;
            inRest = true;
            inCandidates = false;
        }
        else if (cmd[line] == "Began")
        {
            if (inRest)
            {
                cnt.endTime = time[line];
                cnt.type = Rest;
                span.push_back(cnt);
                inRest = false;
                cnt.startTime = time[line];
            }
            if (cnt.startTime == -1)
                cnt.startTime = time[line];
        }
        else if (cmd[line] == "Stationary" || cmd[line] == "Moved")
        {

        }
        else if (cmd[line] == "Ended")
            cnt.endTime = time[line];
        else if (cmd[line] == "NextCandidatePanel" || cmd[line] == "PhraseEnd")
        {

        }
        else
            cout << "Unknown cmd: " << cmd[line] << endl;
    }
    memset(timeCount, 0, sizeof(timeCount));
    rep(i, span.size())
    {
        timeCount[span[i].type] += span[i].endTime - span[i].startTime;
        timeBlock[span[i].type] += span[i].endTime - span[i].startTime;
    }
    timeFout << userID << ","
             << algorithm << ","
             << (id / 6) % 8 + 1 << ","
             << mode[id] << ","
             << sentence[id];
    rep(i, TYPENUM)
        timeFout << "," << timeCount[i];
    timeFout << endl;

    if ((id+1) % 6 == 0)
    {
        double tot = 0;
        rep(i, TYPENUM)
            tot += timeBlock[i];
        timeRatioFout << userID << ","
                      << algorithm << ","
                      << (id / 6) % 8 + 1 << ","
                      << mode[id];
        rep(i, TYPENUM)
            timeRatioFout << "," << timeBlock[i] / tot;
        timeRatioFout << endl;
        memset(timeBlock, 0, sizeof(timeBlock));
    }


    /*
    rep(i, span.size())
    {
        cout << typeToString(span[i].type) << "\t"
             << span[i].startTime << " - " << span[i].endTime << " ";
        if (span[i].type == Select)
            cout << "\t" << span[i].n << " " << span[i].para;
        if (span[i].type == Delete)
            cout << "\t" << span[i].para;
        puts("");
    }
    puts("");
    */
}

void outputWPM(int id)
{
    //double maxWPM[2] = {0, 0};
    int deleteCnt = 0, cancelCnt = 0, correct = 0, uncorrected = 0;
    rep(i, span.size())
        if (span[i].type == Delete)
            deleteCnt++;
        else if (span[i].type == Cancel)
            cancelCnt++;

    vector<string> inputWords;
    sentenceToWords(userText[id], inputWords);
    rep(i, words.size())
        if (i >= inputWords.size() || words[i] != inputWords[i])
            uncorrected++;
        else
            correct++;

    WPMFout << userID << ","
            << algorithm << ","
            << (id / 6) % 8 + 1 << ","
            << mode[id] << ","
            << sentence[id] << "," << WPM[id] << ","
            << correct << "," << uncorrected << ","
            << cancelCnt << "," << deleteCnt
            << endl;
}

void outputCandidates(int id)
{
    vector<int> tops;
    rep(i, span.size())
        if (span[i].type == Select)
            tops.push_back(span[i].n);
        else if (span[i].type == Delete && !tops.empty())
            tops.pop_back();
        else if (span[i].type == Cancel)
            top[12]++;
    rep(i, tops.size())
        top[tops[i]]++;

    if ((id+1) % 6 == 0)
    {
        For(i, 12)
            top[i] += top[i-1];
        candFout << userID << ","
                 << algorithm << ","
                 << (id / 6) % 8 + 1 << ","
                 << mode[id];
        rep(i, 13)
            candFout << "," << top[i];
        rep(i, 12)
            candFout << "," << (float)top[i] / top[12];
        candFout << endl;
        memset(top, 0, sizeof(top));
    }

}

int main()
{
    initFstream();
    rep(p, USER_NUM)
    {
        name = user[p];
        userID = id[p];
        //if (p < 6)
        algorithm = "DTW";
        rep(i, PHRASES)
        {
            readData(i);
            calcTimeDistribution(i);
            outputWPM(i);
            outputCandidates(i);
        }
    }
    return 0;
}

