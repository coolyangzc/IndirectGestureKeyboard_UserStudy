#include "Common.h"
#include "Vector2.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>

const int LEXICON_SIZE = 50000;
const int SAMPLE_NUM = 40;

const int LAYOUT = 0;

// 1:1
//const float keyHeight = 1.0;
//const float keyWidth = 1.0;


//1:3
const float keyHeight = 3.0;
const float keyWidth = 1.0;


using namespace std;

const string keys[3] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
Vector2 keyPos[2][26];

double dtw[MAXSAMPLE][MAXSAMPLE];

string word[LEXICON_SIZE], origin_word[LEXICON_SIZE];
int freq[LEXICON_SIZE];

float x[LEXICON_SIZE][SAMPLE_NUM], y[LEXICON_SIZE][SAMPLE_NUM];
vector<Vector2> pts[LEXICON_SIZE];

pair<float, char> clarity[26];

void initKeyLayout()
{
    rep(i, keys[0].size())
    {
        int key = keys[0][i] - 'a';
        keyPos[0][key] = keyPos[1][key] = Vector2((i + 0.5) * keyWidth, 0.5 * keyHeight);
    }
    rep(i, keys[1].size())
    {
        int key = keys[1][i] - 'a';
        keyPos[0][key] = keyPos[1][key] = Vector2((i + 1.0) * keyWidth, 1.5 * keyHeight);
    }
    rep(i, keys[2].size())
    {
        int key = keys[2][i] - 'a';
        keyPos[0][key] = Vector2((i + 1.5) * keyWidth, 2.5 * keyHeight);
        keyPos[1][key] = Vector2((i + 2.0) * keyWidth, 2.5 * keyHeight);
    }
}

void initLexicon()
{
    fstream fin;
    fin.open("corpus-written-noduplicate.txt", fstream::in);
    rep(i, LEXICON_SIZE)
        fin >> word[i] >> freq[i];
    fin.close();
}

vector<Vector2> wordToPath(string word, const Vector2* keyPos)
{
    vector<Vector2> pts;
    int preKey = -1;
    rep(i, word.length())
    {
        int key = word[i] - 'a';
        if (key != preKey)
            pts.push_back(Vector2(keyPos[key].x, keyPos[key].y));
        preKey = key;
    }
    return pts;
}


float calcGestureClarity(int lexicon_size, const Vector2* keyPos)
{
    rep(i, lexicon_size)
    {
        vector<Vector2> path = wordToPath(word[i], keyPos);
        pts[i] = temporalSampling(path, SAMPLE_NUM);
    }
    double clarity = 0, totFreq = 0;
    rep(i, lexicon_size)
    {
        if (i % 1000 == 0)
            cout << (i / 100) << "% ";
        double best = inf;
        rep(j, lexicon_size)
        {
            if (i == j) continue;
            if (dist(pts[i][0], pts[j][0]) > 1.9 ||
                dist(pts[i][SAMPLE_NUM - 1], pts[j][SAMPLE_NUM - 1]) > 1.9)
                continue;
            double now = match(pts[i], pts[j], dtw, Standard, best);
            best = min(best, now);
        }
        totFreq += freq[i];
        clarity += best * freq[i];
    }
    clarity /= totFreq * SAMPLE_NUM;
    return clarity;
}

bool cmp(const pair<float, char> &a, const pair<float, char> &b)
{
    if (a.first > b.first)
        return true;
    else
        return false;
}

int main()
{
    initKeyLayout();
    initLexicon();

    cout << calcGestureClarity(10000, keyPos[LAYOUT]) << endl;
    rep(i, LEXICON_SIZE)
        origin_word[i] = word[i];
    rep(i, 26)
    {
        char ch = i + 'a';
        rep(j, 10000)
            word[j] = ch + origin_word[j];
        clarity[i] = make_pair(calcGestureClarity(10000, keyPos[LAYOUT]), ch);
        cout << ch << ":" << clarity[i].first << endl;
    }
    cout << endl;
    sort(clarity, clarity + 26, cmp);
    rep(i, 26)
        cout << clarity[i].second << ":" << clarity[i].first << endl;
    return 0;
}
