#include "Common.h"
#include "Vector2.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>

const int LEXICON_SIZE = 10000;
const int SAMPLE_NUM = 40;
const int NEIGHBOR_NUM = 13;

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

vector<Vector2> pts[LEXICON_SIZE];

pair<vector<double>, char> precisionOfTops[26];

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

void initDTW(double dtw[MAXSAMPLE][MAXSAMPLE])
{
    rep(i, MAXSAMPLE)
        rep(j, MAXSAMPLE)
            dtw[i][j] = inf;
    dtw[0][0] = 0;
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

void updatePrecision(int i, int j, vector<double>& precision)
{
    double now = match(pts[i], pts[j], dtw, DTW, precision.back());
    if (now >= precision.back()) return;
    rep(k, NEIGHBOR_NUM)
        if (now <= precision[k])
        {
            FORD(l, NEIGHBOR_NUM - 1, k + 1)
                precision[l] = precision[l - 1];
            precision[k] = now;
            return;
        }
}

vector<double> calcPrecisionOfTops(int lexicon_size, const Vector2* keyPos)
{
    rep(i, lexicon_size)
    {
        vector<Vector2> path = wordToPath(word[i], keyPos);
        pts[i] = temporalSampling(path, SAMPLE_NUM);
    }
    double totFreq = 0;
    vector<double> precision(NEIGHBOR_NUM);
    vector<double> sumPrecision(NEIGHBOR_NUM);
    vector<int> pruning_id;
    float pruning_diff = 3.0f;
    rep(i, lexicon_size)
    {
        if (i % 1000 == 0)
            cout << (i / 100) << "% ";
        rep(j, NEIGHBOR_NUM)
            precision[j] = 1e10;
        pruning_id.clear();
        rep(j, lexicon_size)
        {
            if (i == j) continue;
            if (dist(pts[i][0], pts[j][0]) > pruning_diff ||
                dist(pts[i][SAMPLE_NUM - 1], pts[j][SAMPLE_NUM - 1]) > pruning_diff)
                pruning_id.push_back(j);
            else
                updatePrecision(i, j, precision);
        }
        if (precision.back() == 1e10)
        {
            rep(j, pruning_id.size())
                updatePrecision(i, pruning_id[j], precision);
        }
        totFreq += freq[i];
        rep(j, NEIGHBOR_NUM)
            sumPrecision[j] += precision[j] * freq[i];
    }
    rep(i, NEIGHBOR_NUM)
        sumPrecision[i] /= totFreq;
    return sumPrecision;
}

int main()
{
    initKeyLayout();
    initLexicon();
    initDTW(dtw);

    rep(i, LEXICON_SIZE)
        origin_word[i] = word[i];

    vector<double> origin_p = calcPrecisionOfTops(LEXICON_SIZE, keyPos[LAYOUT]);

    rep(i, 26)
    {
        char ch = i + 'a';
        rep(j, LEXICON_SIZE)
            word[j] = ch + origin_word[j];
        precisionOfTops[i] = make_pair(calcPrecisionOfTops(LEXICON_SIZE, keyPos[LAYOUT]), ch);
    }

    cout << endl << "N/A: ";
    rep(i, NEIGHBOR_NUM)
        cout << " " << origin_p[i];
    puts("");
    rep(i, 26)
    {
        cout << precisionOfTops[i].second << ":";
        rep(j, NEIGHBOR_NUM)
            cout << " " << precisionOfTops[i].first[j];
        puts("");
    }
}
