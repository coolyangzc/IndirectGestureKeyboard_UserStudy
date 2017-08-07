#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include "opencv2/opencv.hpp"

#define sqr(x) ((x)*(x))
#define rep(i,n) for(int i=0; i<n; i++)
#define length(x,y) sqrt((x)*(x) + (y)*(y))

const int LEXICON_SIZE = 50000;


// 1:1
const int H = 720;
const int W = 1280;
const double SCALE = 120.0d;
const double keyHeight = 1.0;
const double keyWidth = 1.0;
const double fontDX = keyWidth * 0.11d * SCALE;
const double fontDY = keyHeight * 0.11d * SCALE;


/*
//1:3
const int H = 720;
const int W = 1280;
const double SCALE = 70.0d;
const double keyHeight = 3.0;
const double keyWidth = 1.0;
const double fontDX = keyWidth * 0.2d * SCALE;
const double fontDY = keyHeight * 0.11d * SCALE;
*/

const double X_OFFSET = (W - keyWidth * 10 * SCALE) / 2;
const double Y_OFFSET = (H - keyHeight * 3 * SCALE) / 2;

using namespace cv;
using namespace std;

const string keys[3] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
float keyX[26], keyY[26];


string dict[LEXICON_SIZE];
int freq[LEXICON_SIZE];

void drawLine(Mat& img, Point u, Point v, int thickness = 5)
{
    line(img, u, v, Scalar(0,255,255), thickness, CV_AA);
    circle(img, u, 10, Scalar(0, 0, 255));
}

void drawCross(Mat& img, double x, double y, const Scalar& color)
{
    double d = SCALE / 8;
    line(img, Point(x-d,y-d), Point(x+d,y+d), color, 2, CV_AA);
    line(img, Point(x+d,y-d), Point(x-d,y+d), color, 2, CV_AA);
}

void initKeyLayout(Mat& img, double keyWidth, double keyHeight)
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

    double H2 = keyHeight * 0.5d * SCALE, W2 = keyWidth * 0.5d * SCALE;
    rep(key, 26)
    {
        keyX[key] = keyX[key] * SCALE + X_OFFSET;
        keyY[key] = keyY[key] * SCALE + Y_OFFSET;
        rectangle(img, Point(keyX[key] - W2, keyY[key] - H2),
                       Point(keyX[key] + W2, keyY[key] + H2), Scalar(0,0,0), 1, CV_AA);
        char ch = key + 'A';
        stringstream stream;
        stream << ch;
        string str = stream.str();
        putText(img, str, Point(keyX[key] - fontDX, keyY[key] + fontDY), FONT_HERSHEY_DUPLEX , 1.4, Scalar(0,0,0), 1,CV_AA);
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

void calcGeometricMedian(Mat& img, int lexicon_size)
{
    double weight[26];
    memset(weight, 0, sizeof(weight));

    rep(i, lexicon_size)
        weight[dict[i][0] - 'a'] += freq[i];
    double x[2], y[2], sumWeight, inv;
    x[0] = y[0] = 0;
    rep(i, 26)
    {
        printf("%c:%.0f\n", i + 'a', weight[i]);
        x[0] += keyX[i] * weight[i];
        y[0] += keyY[i] * weight[i];
        sumWeight += weight[i];
    }
    x[0] /= sumWeight; y[0] /= sumWeight;
    drawCross(img, x[0], y[0], Scalar(0, 0, 255));
    rep(iteration, 10000)
    {
        x[1] = y[1] = inv = 0;
        rep(i, 26)
        {
            double dist = length(x[0]-keyX[i], y[0]-keyY[i]);
            inv += weight[i] / dist;
            x[1] += keyX[i] * weight[i] / dist;
            y[1] += keyY[i] * weight[i] / dist;
        }
        x[1] /= inv; y[1] /= inv;
        x[0] = x[1];
        y[0] = y[1];
    }
    drawCross(img, x[0], y[0], Scalar(255, 0, 0));

}

void draw(string outputFileName, bool show = false)
{
    Mat img = Mat(H, W, CV_8UC3, Scalar(255, 255, 255));
    initKeyLayout(img, keyWidth, keyHeight);
    initLexicon();
    calcGeometricMedian(img, LEXICON_SIZE);
    imshow("Gesture Drawer", img);
    imwrite(outputFileName, img);
}

int main()
{
    //freopen("time.txt", "r", stdin);
    //drawFile("time.txt");
    draw("out.jpg", true);
    waitKey();
    return 0;
}
