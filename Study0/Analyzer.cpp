#include <vector>
#include <string>
#include <cstdio>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#define rep(i,n) for(int i=0; i<n; i++)

using namespace std;

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
        if (s[0] == 'E')
            break;
    }
    sort(x.begin(), x.end());
    sort(y.begin(), y.end());

    int id = x.size() * 0.10f;
    double minX = x[id], maxX = x[x.size() - id - 1], minY = y[id], maxY = y[y.size() - id - 1];
    X = (minX + maxX) * 0.5 / W;
    Y = (minY + maxY) * 0.5 / H;
    H = (maxY - minY) / H;
    W = (maxX - minX) / W;
    fout << X << "," << Y << "," << H << "," << W << endl;
}

int main()
{
    qnFin.open("data/Questionnaire_Data.csv");
    qnFin >> line;
    fout.open("Study0.csv", fstream::out);
    fout << line << ",Phone,Usage,X,Y,Height,Width" << endl;
    rep(i, 12)
    {
        qnFin >> line;
        stringstream ss_id;
        ss_id << i;
        rep(j, 10)
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
