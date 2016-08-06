#include <cstdio>
#include <fstream>
#include <iostream>
#include "opencv2/opencv.hpp"
#define rep(i,n) for(int i=0; i<n; i++)

int H, W;

using namespace cv;
using namespace std;

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
    //circle(img, v, 10, Scalar(0, 0, 255));
}

void draw(string outputFileName)
{
    string s, buffer;
    double X, Y, T;
    vector<double> x, y;
    int cnt = 0;
    cin >> H >> W;
    while (cin >> s)
    {
        x.clear();
        y.clear();
        buffer = "";
        while (true)
        {
            scanf("%lf%lf%lf", &T, &X, &Y);
            Y = H - Y;
            x.push_back(X);
            y.push_back(Y);
            stringstream sX, sY;
            sX << X; sY << Y;
            buffer += s + " " + sX.str() + " " + sY.str() + "\n";
            if (s[0] == 'E')
                break;
            cin >> s;
        }
        if (x.size() <= 30)
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
        drawLine(img, Point(minX, minY), Point(minX, maxY), 8, &color);
        drawLine(img, Point(minX, minY), Point(maxX, minY), 8, &color);
        drawLine(img, Point(maxX, maxY), Point(minX, maxY), 8, &color);
        drawLine(img, Point(maxX, maxY), Point(maxX, minY), 8, &color);
        stringstream ss;
        cout << cnt << ":" << x.size() << endl;
        ss << cnt++;
        imwrite(outputFileName + "_" + ss.str() + ".jpg", img);
        string fileName = outputFileName + "_" + ss.str() + ".txt";
        fstream fout;
        fout.open(fileName.c_str(), fstream::out);
        fout << buffer << endl;
    }
    puts("Finish");
}

void Merge(Mat& img, fstream& fin, Scalar color)
{
    string s;
    double X, Y;
    vector<double> x, y;
    int cnt = 0;
    double minX = 0, maxX = 0, minY = 0, maxY = 0;
    while (fin >> s)
    {
        x.clear();
        y.clear();
        while (true)
        {
            fin >> X >> Y;
            Y = H - Y;
            x.push_back(X);
            y.push_back(Y);
            if (s[0] == 'E')
                break;
            fin >> s;
        }
        rep(i, x.size() - 1)
        {
            //drawLine(img, Point(x[i], y[i]), Point(x[i+1], y[i+1]), 4, &color);
        }
        sort(x.begin(), x.end());
        sort(y.begin(), y.end());

        int id = x.size() * 0.10f;
        minX += x[id], maxX += x[x.size() - id - 1], minY += y[y.size() - id - 1], maxY += y[id];
        //drawLine(img, Point(x[id], y[y.size() - id - 1]), Point(x[id], y[id]), 3, &color);
        //drawLine(img, Point(x[id], y[y.size() - id - 1]), Point(x[x.size() - id - 1], y[y.size() - id - 1]), 3, &color);
        //drawLine(img, Point(x[x.size() - id - 1], y[id]), Point(x[id], y[id]), 3, &color);
        //drawLine(img, Point(x[x.size() - id - 1], y[id]), Point(x[x.size() - id - 1], y[y.size() - id - 1]), 3, &color);
        cnt++;
    }
    minX /= cnt;
    maxX /= cnt;
    minY /= cnt;
    maxY /= cnt;
    CX[0] += minX;
    CX[1] += maxX;
    CY[0] += minY;
    CY[1] += maxY;

}

int main()
{
    freopen("data/raw/1440P/6.txt", "r", stdin);
    draw("6");

    //string name[] = {"maye", "yixin", "yxc", "mzy", "xwj", "yuntao"};
    //int User = 6;

    /*string file = name[4];
    string dir = "data/refine/" + file + ".txt";
    freopen(dir.c_str(), "r", stdin);
    draw(file.c_str());
    fclose(stdin);*/


    //freopen("maye2.txt", "r", stdin);
    //draw("out");

    /*Mat img = Mat::zeros(H, W, CV_8UC3);
    rep(i, User)
    {
        Scalar color(Random(255), Random(255), Random(255));

        stringstream ss;
        fstream fin;
        string file = "data/refine/" + name[i] + ".txt";
        fin.open(file.c_str(), fstream::in );
        cout << file << endl;
        Merge(img, fin, color);
        fin.close();

    }
    Scalar red = Scalar(0, 0, 255);
    double minX = CX[0] / User, maxX = CX[1] / User, minY = CY[0] / User, maxY = CY[1] / User;
    drawLine(img, Point(minX, minY), Point(minX, maxY), 8, &red);
    drawLine(img, Point(minX, minY), Point(maxX, minY), 8, &red);
    drawLine(img, Point(maxX, maxY), Point(minX, maxY), 8, &red);
    drawLine(img, Point(maxX, maxY), Point(maxX, minY), 8, &red);
    imwrite("Merge.jpg", img);*/

    //waitKey();
    return 0;
}
