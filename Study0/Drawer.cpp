#include <cstdio>
#include <fstream>
#include <iostream>
#include "opencv2/opencv.hpp"
#define rep(i,n) for(int i=0; i<n; i++)
#define FOR(i,a,b) for(int i=a; i<=b; i++)

using namespace cv;
using namespace std;

const double DELTA = 1;

int H, W, cnt;
double CX[2], CY[2];

int Random(int mo)
{
    return rand() % mo;
}

void drawLine(Mat& img, Point u, Point v, int thickness = 5, Scalar* color = NULL)
{
    int lineType = 8;
    if (!color)
        color = new Scalar(0, 255, 255);
    line(img, u, v, *color, thickness, lineType);
    //circle(img, v, 15, Scalar(0, 0, 255), 4);
}

void draw(fstream& fin, string outputFileName)
{
    string s, buffer;
    double X, Y, T;
    vector<double> x, y;
    int cnt = 0;
    fin >> H >> W;
    while (fin >> s)
    {
        x.clear();
        y.clear();
        double PreX = -1, PreY = -1;
        buffer = "";
        while (true)
        {
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
            if (s[0] == 'E')
                break;
            fin >> s;
        }
        if (x.size() <= 28)
        {
            cout << x.size() << endl;
            continue;
        }

        Mat img = Mat::zeros(H, W, CV_8UC3);
        rep(i, x.size() - 1)
            drawLine(img, Point(x[i], y[i]), Point(x[i+1], y[i+1]));

        sort(x.begin(), x.end());
        sort(y.begin(), y.end());

        int id = x.size() * 0.10f;
        double minX = x[id], maxX = x[x.size() - id - 1], minY = y[y.size() - id - 1], maxY = y[id];
        Scalar color = Scalar(0, 0, 255);
        drawLine(img, Point(minX, minY), Point(minX, maxY), 6, &color);
        drawLine(img, Point(minX, minY), Point(maxX, minY), 6, &color);
        drawLine(img, Point(maxX, maxY), Point(minX, maxY), 6, &color);
        drawLine(img, Point(maxX, maxY), Point(maxX, minY), 6, &color);
        stringstream ss;
        cout << cnt << ":" << x.size() << endl;
        ss << cnt++;
        imwrite(outputFileName + "_" + ss.str() + ".jpg", img);
        string fileName = outputFileName + "_" + ss.str() + ".txt";
        fstream fout;
        fout.open(fileName.c_str(), fstream::out);
        fout << H << endl << W << endl;
        fout << buffer << endl;
    }
    puts("Finish");
}

void Merge(Mat& img, fstream& fin, Scalar color)
{
    string s;
    double X, Y;
    vector<double> x, y;
    fin >> H >> W;
    while (fin >> s)
    {
        fin >> X >> Y;
        x.push_back(X);
        y.push_back(Y);
        if (s[0] == 'E')
            break;
    }
    sort(x.begin(), x.end());
    sort(y.begin(), y.end());

    int id = x.size() * 0.10f;
    double minX = x[id], maxX = x[x.size() - id - 1], minY = y[id], maxY = y[y.size() - id - 1];
    drawLine(img, Point(minX, minY), Point(minX, maxY), 4, &color);
    drawLine(img, Point(minX, minY), Point(maxX, minY), 4, &color);
    drawLine(img, Point(maxX, maxY), Point(minX, maxY), 4, &color);
    drawLine(img, Point(maxX, maxY), Point(maxX, minY), 4, &color);
    cnt++;
    CX[0] += minX;
    CX[1] += maxX;
    CY[0] += minY;
    CY[1] += maxY;

}

void DrawMerge()
{
    Mat img = Mat::zeros(2560, 1440, CV_8UC3);
    rep(i, 12)
    {
        Scalar color(Random(255), Random(255), Random(255));
        stringstream ss_id;
        ss_id << i;
        FOR(j, 0, 4)
        {
            stringstream ss_num;
            ss_num << j;
            fstream fin;
            string fileName = "res/1440P/" + ss_id.str() + "_" + ss_num.str() + ".txt";
            fin.open(fileName.c_str(), fstream::in );
            Merge(img, fin, color);
            fin.close();
        }
    }
    Scalar red = Scalar(0, 0, 255);
    double minX = CX[0] / cnt, maxX = CX[1] / cnt, minY = CY[0] / cnt, maxY = CY[1] / cnt;
    drawLine(img, Point(minX, minY), Point(minX, maxY), 16, &red);
    drawLine(img, Point(minX, minY), Point(maxX, minY), 16, &red);
    drawLine(img, Point(maxX, maxY), Point(minX, maxY), 16, &red);
    drawLine(img, Point(maxX, maxY), Point(maxX, minY), 16, &red);
    imwrite("Merge.jpg", img);
}

int main()
{

    DrawMerge();
    /*FOR(i, 0, 11)
    {
        stringstream ss;
        ss << i;
        string fileName = "data/refine/1440P/" + ss.str() + ".txt";
        cout << fileName << endl;
        fstream fin;
        fin.open(fileName.c_str());
        draw(fin, ss.str());
    }*/
    //freopen("data/raw/720P/10.txt", "r", stdin);
    //draw("10");

    //string name[] = {"maye", "yixin", "yxc", "mzy", "xwj", "yuntao"};
    //int User = 6;

    /*string file = name[4];
    string dir = "data/refine/" + file + ".txt";
    freopen(dir.c_str(), "r", stdin);
    draw(file.c_str());
    fclose(stdin);*/


    //freopen("maye2.txt", "r", stdin);
    //draw("out");



    //waitKey();    return 0;
}
