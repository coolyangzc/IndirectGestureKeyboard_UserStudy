#include <vector>
#include <string>
#include <cstdio>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#define rep(i,n) for(int i=0; i<n; i++)
#define For(i,n) for(int i=1; i<=n; i++)

using namespace std;

const double W_720P = 53.546, H_720P = 95.193;
const double W_1440P = 63.508, H_1440P = 112.904;

double H, W;
string line;
fstream qnFin, fout;

void Analyse(fstream& fin)
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
    }
    sort(x.begin(), x.end());
    sort(y.begin(), y.end());

    int id = x.size() * 0.10f;
    double minX = x[id], maxX = x[x.size() - id - 1], minY = y[id], maxY = y[y.size() - id - 1];
    double dist = 0, height = 0, width = 0;
    bool is720P = false;
    if (W == 720)
        is720P = true;
    X = (minX + maxX) * 0.5 / W;
    Y = (minY + maxY) * 0.5 / H;
    H = (maxY - minY) / H;
    W = (maxX - minX) / W;
    if (is720P)
    {
        dist = (1 - X) * W_720P;
        height = H * H_720P;
        width = W * W_720P;
    }
    else
    {
        dist = (1 - X) * W_1440P;
        height = H * H_1440P;
        width = W * W_1440P;
    }

    fout << X << "," << Y << "," << H << "," << W << ","
         << height << "," << width << "," << dist << endl;
}

int main()
{
    qnFin.open("data/Questionnaire_Data.csv");
    qnFin >> line;
    fout.open("Study2.csv", fstream::out);
    fout << line << ",Phone,Usage,X,Y,Height,Width,Height(mm),Width(mm),Dist2Right(mm)" << endl;
    For(i, 16)
    {
        qnFin >> line;
        stringstream ss_id;
        ss_id << i;
        For(j, 10)
        {
            stringstream ss_num;
            ss_num << j;
            fout << line << ",720P," << ((j<5)?"Thumb,":"Index,");
            string fileName = "res/720P/" + ss_id.str() + "_" + ss_num.str() + ".txt";
            fstream fin;
            fin.open(fileName.c_str());
            Analyse(fin);
            fout << line << ",1440P," << ((j<5)?"Thumb,":"Index,");
            fileName = "res/1440P/" + ss_id.str() + "_" + ss_num.str() + ".txt";
            fin.close();
            fin.open(fileName.c_str());
            Analyse(fin);
        }
    }
}
