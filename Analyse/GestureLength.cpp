#include "Common.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>

#define sqr(x) ((x)*(x))
#define len(x,y) sqrt((x)*(x) + (y)*(y))

const int LEXICON_SIZE = 10000;

/*
// 1:1
const int H = 720;
const int W = 1280;
const double keyHeight = 1.0;
const double keyWidth = 1.0;
*/


//1:3
const int H = 720;
const int W = 1280;
const double keyHeight = 3.0;
const double keyWidth = 1.0;

using namespace std;

const string keys[3] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
float keyX[26], keyY[26];

string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];
double normalLength, sumWeight;
double weight[26];

void initKeyLayout(double keyWidth, double keyHeight)
{
    rep(i, keys[0].size())
    {
        int key = keys[0][i] - 'a';
        keyX[key] = (i + 0.5d) * keyWidth;
        keyY[key] = 0.5d * keyHeight;
    }
    rep(i, keys[1].size())
    {
        int key = keys[1][i] - 'a';
        keyX[key] = (i + 1.0d) * keyWidth;
        keyY[key] = 1.5d * keyHeight;
    }
    rep(i, keys[2].size())
    {
        int key = keys[2][i] - 'a';
        keyX[key] = (i + 1.5d) * keyWidth;
        keyY[key] = 2.5d * keyHeight;
    }
}

void initLexicon()
{
    fstream fin;
    fin.open("corpus-written-noduplicate.txt", fstream::in);
    string s;
    rep(i, LEXICON_SIZE)
        fin >> dict[i] >> freq[i];
    fin.close();
}

void initNormalGestureLength()
{
    rep(i, LEXICON_SIZE)
        sumWeight += freq[i];
    normalLength = 0;
    rep(i, LEXICON_SIZE)
    {
        double dis = 0;
        rep(j, dict[i].length() - 1)
            dis += len(keyX[dict[i][j] - 'a'] - keyX[dict[i][j+1] - 'a'],
                       keyY[dict[i][j] - 'a'] - keyY[dict[i][j+1] - 'a']);
        normalLength += dis * freq[i];
    }
    normalLength /= sumWeight;
    normalLength /= keyWidth;
}

double calc(double preX, double preY)
{
    double tot = 0;
    rep(i, 26)
        tot += len(keyX[i] - preX, keyY[i] - preY) * weight[i];
    return tot / sumWeight / keyWidth + normalLength;
}

void calcGestureLength()
{
    memset(weight, 0, sizeof(weight));
    rep(i, LEXICON_SIZE)
        weight[dict[i][0] - 'a'] += freq[i];
    double x[2], y[2], inv;
    x[0] = y[0] = 0;
    rep(i, 26)
    {
        printf("%c:%.0f\n", i + 'a', weight[i]);
        x[0] += keyX[i] * weight[i];
        y[0] += keyY[i] * weight[i];
    }
    x[0] /= sumWeight; y[0] /= sumWeight;
    rep(iteration, 10000)
    {
        x[1] = y[1] = inv = 0;
        rep(i, 26)
        {
            double dist = len(x[0]-keyX[i], y[0]-keyY[i]);
            inv += weight[i] / dist;
            x[1] += keyX[i] * weight[i] / dist;
            y[1] += keyY[i] * weight[i] / dist;
        }
        x[1] /= inv; y[1] /= inv;
        x[0] = x[1];
        y[0] = y[1];
    }
    printf("Normal = %lf\n", normalLength);
    printf("Best = %lf\n", calc(x[0], y[0]));
    printf("G-Keyboard = %lf\n", calc(keyX['g' - 'a'], keyY['g' - 'a']));
    rep(i, 26)
    {
        double d = calc(keyX[i], keyY[i]);
        printf("%c: %lf\n", i + 'a', (d - normalLength) / normalLength);
    }
    rep(i, 26)
    {
        double d = calc(keyX[i], keyY[i]);
        printf("[%.1lf,%.1lf,\"%.1lf\"],", -keyY[i] / keyHeight, keyX[i] / keyWidth, (d - normalLength) / normalLength * 100);
    }
}

void calcForHeatMap()
{
    FILE *fout;
    fout = fopen("res/length.txt", "w+");
    int X = 10, Y = 30;
    double dx = keyWidth / X, dy = keyHeight / Y;
    rep(c, 26)
    {
        rep(i, X)
            rep(j, Y)
            {
                double x = keyX[c] - keyWidth / 2 + (i + 0.5f) * dx;
                double y = keyY[c] - keyHeight / 2 + (j + 0.5f) * dy;
                double d = calc(x, y);
                int px = x / keyWidth * X;
                int py = y / keyHeight * Y;
                fprintf(fout, "[%d,%d,\"%.2lf\"],", -py, px, (d - normalLength) / normalLength * 100);
            }
    }
    fclose(fout);
}

int main()
{
    initLexicon();
    initKeyLayout(keyWidth, keyHeight);
    initNormalGestureLength();
    calcGestureLength();
    calcForHeatMap();
    return 0;
}
