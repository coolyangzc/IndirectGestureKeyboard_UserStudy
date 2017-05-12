#include "Common.h"
#include "Vector2.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>

const int LEXICON_SIZE = 50000;
const int SAMPLE_NUM = 50;

// 1:1
const float keyHeight = 1.0;
const float keyWidth = 1.0;

/*
//1:3
const float keyHeight = 3.0;
const float keyWidth = 1.0;
*/

using namespace std;

const string keys[3] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
Vector2 keyPos[2][26];

double dtw[MAXSAMPLE][MAXSAMPLE];

string word[LEXICON_SIZE];
int freq[LEXICON_SIZE];

float x[LEXICON_SIZE][SAMPLE_NUM], y[LEXICON_SIZE][SAMPLE_NUM];
vector<Vector2> pts[LEXICON_SIZE];

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
    fin.open("corpus-written.txt", fstream::in);
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
    rep(i, lexicon_size)
    {
        double dist = inf;
        int best = -1;
        rep(j, lexicon_size)
        {
            if (i == j) continue;
            double now = match(pts[i], pts[j], dtw, Standard, dist);
            if (now < dist)
                dist = now, best = j;
        }
        cout << word[i] << ": " << word[best] << " " << dist / SAMPLE_NUM << endl;

    }

}

int main()
{
    initKeyLayout();
    initLexicon();
    calcGestureClarity(100, keyPos[1]);
    return 0;
}
