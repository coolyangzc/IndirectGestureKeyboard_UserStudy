#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "Common.h"
#include "Vector2.h"

using namespace std;

Vector2 keyPos[26];

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
        keyPos[line3[i] - 'a'] = Vector2(-0.35 + i * 0.1, -0.3333);
    }
}

void initDTW(double dtw[MAXSAMPLE][MAXSAMPLE])
{
    rep(i, MAXSAMPLE)
        rep(j, MAXSAMPLE)
            dtw[i][j] = INF;
    dtw[0][0] = 0;
}

void initKeyboard(double dtw[MAXSAMPLE][MAXSAMPLE])
{
    calcKeyLayout();
    initDTW(dtw);
}

vector<Vector2> wordToPath(string word, float scale)
{
    vector<Vector2> pts;
    int preKey = -1;
    rep(i, word.length())
    {
        int key = word[i] - 'a';
        if (key != preKey)
            pts.push_back(Vector2(keyPos[key].x, keyPos[key].y * 0.3 * scale));
        preKey = key;
    }
    return pts;
}


#endif // KEYBOARD_H_

