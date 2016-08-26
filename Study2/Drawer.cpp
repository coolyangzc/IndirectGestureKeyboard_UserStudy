#include <cstdio>
#include <fstream>
#include <iostream>
#include "opencv2/opencv.hpp"
#define rep(i,n) for(int i=0; i<n; i++)
#define For(i,n) for(int i=1; i<=n; i++)
#define FOR(i,a,b) for(int i=a; i<=b; i++)

using namespace cv;
using namespace std;

const double DELTA = 1;

bool is1440P = false;
int H, W, cnt;
double CX[2], CY[2];

int Random(int mo)
{
    return (rand() << 15 ^ rand()) % mo;
}

void drawLine(Mat& img, Point u, Point v, int thickness = 5, Scalar* color = NULL, bool drawCircle = true)
{
    int lineType = 8;
    if (!color)
        color = new Scalar(0, 255, 255);
    if (is1440P)
    {
        line(img, u, v, *color, thickness * 2, lineType);
        if (!drawCircle) return;
        circle(img, u, 20, Scalar(0, 0, 255), 4);
        circle(img, v, 20, Scalar(0, 0, 255), 4);
    }
    else
    {
        line(img, u, v, *color, thickness, lineType);
        if (!drawCircle) return;
        circle(img, u, 10, Scalar(0, 0, 255), 2);
        circle(img, v, 10, Scalar(0, 0, 255), 2);
    }

}

bool getFormat(string fileName)
{
    fstream fin;
    fin.open(fileName.c_str());
    string s;
    while(fin >> s)
        if (s == "Start")
            return true;
    return false;
}

void draw(fstream& fin, string outputFileName, bool newFormat = true)
{
    if (newFormat) puts("New Format");
              else puts("Old Format");
    string s, buffer, fileBuffer;
    double X, Y, T;
    vector<double> x, y, sx, sy;
    int cnt = 0;
    fin >> H >> W;
    if (W == 720) is1440P = false;
             else is1440P = true;
    while (fin >> s)
    {
        if (newFormat && s != "Start")
            continue;
        Mat img = Mat::zeros(H, W, CV_8UC3);
        x.clear(); y.clear();
        sx.clear(); sy.clear();
        double PreX = -1, PreY = -1;
        buffer = fileBuffer = "";
        while (true)
        {
            if (newFormat)
            {
                fin >> s;
                if (s == "Finish")
                    break;
                fin >> T >> X >> Y;
                Y = H - Y;
                if (fabs(X - PreX) + fabs(Y - PreY) > DELTA)
                {
                    x.push_back(X);
                    y.push_back(Y);
                    stringstream sX, sY;
                    sX << X; sY << Y;
                    PreX = X; PreY = Y;
                    buffer += s + " " + sX.str() + " " + sY.str() + "\n";
                }
                if (s == "Ended")
                {
                    if (x.size() > 10)
                    {
                        rep(i, x.size() - 1)
                            drawLine(img, Point(x[i], y[i]), Point(x[i+1], y[i+1]));
                        rep(i, x.size())
                        {
                            sx.push_back(x[i]);
                            sy.push_back(y[i]);
                        }
                        fileBuffer += buffer;
                    }
                    x.clear(); y.clear();
                    PreX = -1, PreY = -1;
                    buffer = "";
                }
            }
            else
            {
                fin >> T >> X >> Y;
                Y = H - Y;
                if (fabs(X - PreX) + fabs(Y - PreY) > DELTA)
                {
                    sx.push_back(X);
                    sy.push_back(Y);
                    stringstream sX, sY;
                    sX << X; sY << Y;
                    PreX = X; PreY = Y;
                    fileBuffer += s + " " + sX.str() + " " + sY.str() + "\n";
                }
                if (s == "Ended")
                    break;
                fin >> s;
            }
        }

        if (!newFormat && sx.size() <= 25)
            continue;
        if (!newFormat)
            rep(i, sx.size() - 1)
                drawLine(img, Point(sx[i], sy[i]), Point(sx[i+1], sy[i+1]));

        sort(sx.begin(), sx.end());
        sort(sy.begin(), sy.end());

        int id = sx.size() * 0.10f;
        double minX = sx[id], maxX = sx[sx.size() - id - 1], minY = sy[sy.size() - id - 1], maxY = sy[id];
        Scalar color = Scalar(0, 0, 255);
        drawLine(img, Point(minX, minY), Point(minX, maxY), 4, &color, false);
        drawLine(img, Point(minX, minY), Point(maxX, minY), 4, &color, false);
        drawLine(img, Point(maxX, maxY), Point(minX, maxY), 4, &color, false);
        drawLine(img, Point(maxX, maxY), Point(maxX, minY), 4, &color, false);

        stringstream ss;
        cout << ++cnt << endl;
        ss << cnt;
        imwrite(outputFileName + "_" + ss.str() + ".jpg", img);
        string fileName = outputFileName + "_" + ss.str() + ".txt";
        fstream fout;
        fout.open(fileName.c_str(), fstream::out);
        fout << H << endl << W << endl;
        fout << fileBuffer << endl;
    }

    puts("Finish");
}

void Merge(Mat& img, fstream& fin, Scalar color)
{
    string s;
    double X, Y;
    vector<double> x, y;
    fin >> H >> W;
    if (W == 720) is1440P = false;
             else is1440P = true;
    while (fin >> s)
    {
        fin >> X >> Y;
        x.push_back(X);
        y.push_back(Y);
    }
    sort(x.begin(), x.end());
    sort(y.begin(), y.end());

    int id = x.size() * 0.10f;
    double minX = x[id], maxX = x[x.size() - id - 1], minY = y[id], maxY = y[y.size() - id - 1];
    drawLine(img, Point(minX, minY), Point(minX, maxY), 2, &color, false);
    drawLine(img, Point(minX, minY), Point(maxX, minY), 2, &color, false);
    drawLine(img, Point(maxX, maxY), Point(minX, maxY), 2, &color, false);
    drawLine(img, Point(maxX, maxY), Point(maxX, minY), 2, &color, false);
    cnt++;
    CX[0] += minX;
    CX[1] += maxX;
    CY[0] += minY;
    CY[1] += maxY;

}

void DrawMerge(int num)
{
    Mat img = Mat::zeros(1280, 720, CV_8UC3);
    For(i, num)
    {
        Scalar color(Random(255), Random(255), Random(255));
        stringstream ss_id;
        ss_id << i;
        FOR(j, 1, 5)
        {
            stringstream ss_num;
            ss_num << j;
            fstream fin;
            string fileName = "res/720P/" + ss_id.str() + "_" + ss_num.str() + ".txt";
            fin.open(fileName.c_str(), fstream::in );
            Merge(img, fin, color);
            fin.close();
        }
    }
    Scalar red = Scalar(0, 0, 255);
    double minX = CX[0] / cnt, maxX = CX[1] / cnt, minY = CY[0] / cnt, maxY = CY[1] / cnt;
    drawLine(img, Point(minX, minY), Point(minX, maxY), 8, &red, false);
    drawLine(img, Point(minX, minY), Point(maxX, minY), 8, &red, false);
    drawLine(img, Point(maxX, maxY), Point(minX, maxY), 8, &red, false);
    drawLine(img, Point(maxX, maxY), Point(maxX, minY), 8, &red, false);
    imwrite("Merge.jpg", img);
}

int main()
{
    //DrawMerge(16);
    FOR(i, 16, 16)
    {
        stringstream ss;
        ss << i;
        string fileName = "data/refine/720P/" + ss.str() + ".txt";
        cout << fileName << endl;
        fstream fin;
        fin.open(fileName.c_str());
        draw(fin, ss.str(), getFormat(fileName));
    }
    return 0;
}
