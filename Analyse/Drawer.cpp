#include <cstdio>
#include <iostream>
#include "opencv2/opencv.hpp"
#define rep(i,n) for(int i=0; i<n; i++)

const int H = 600;
const int W = 1200;
const double X_OFFSET = 150;
const double Y_OFFSET = 100;
const double SCALE = 5.0d;
const double keyHeight = 25.0;
const double keyWidth = 18.0;
const int MAX_POINTS = 1000;

using namespace cv;
using namespace std;

const string keys[3] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
float keyX[26], keyY[26];

int n;
int time[MAX_POINTS];
double x[MAX_POINTS], y[MAX_POINTS];

void drawLine(Mat& img, Point u, Point v, int thickness = 5)
{
    int lineType = 8;
    line(img, u, v, Scalar(0,255,255), thickness, lineType);
    circle(img, u, 10, Scalar(0, 0, 255));
}

void drawKeyLayout(Mat& img)
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
        keyX[key] = (i + 2.0d) * keyWidth;
        keyY[key] = 2.5d * keyHeight;
    }

    double H2 = keyHeight * 0.5d * SCALE, W2 = keyWidth * 0.5d * SCALE;
    double DX = keyHeight * 0.06d * SCALE, DY = keyWidth * 0.1d * SCALE;
    rep(key, 26)
    {
        keyX[key] = keyX[key] * SCALE + X_OFFSET;
        keyY[key] = keyY[key] * SCALE + Y_OFFSET;
        rectangle(img, Point(keyX[key] - W2, keyY[key] - H2),
                       Point(keyX[key] + W2, keyY[key] + H2), Scalar(255,255,255));
        char ch = key + 'A';
        stringstream stream;
        stream << ch;
        string str = stream.str();
        putText(img, str, Point(keyX[key] - DX, keyY[key] + DY), 0, 1.4, Scalar(255,255,255));
    }

}

void draw(string outputFileName, bool show = false)
{
    Mat img = Mat::zeros(H, W, CV_8UC3);

    drawKeyLayout(img);
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
