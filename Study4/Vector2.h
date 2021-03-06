#ifndef VECTOR2_H_
#define VECTOR2_H_

#include "Common.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

struct Vector2
{
    double x, y;
    Vector2(double x_ = 0, double y_ = 0)
    {
        x = x_;
        y = y_;
    }
    Vector2 operator +(const Vector2 &b)
    {
        return Vector2(x + b.x, y + b.y);
    }
    Vector2 operator -(const Vector2 &b)
    {
        return Vector2(x - b.x, y - b.y);
    }
    Vector2 operator *(const double &r)
    {
        return Vector2(x*r, y*r);
    }
    Vector2 operator /(const double &r)
    {
        return Vector2(x/r, y/r);
    }
};

double dist(const Vector2& p, const Vector2& q)
{
    return sqrt((p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y));
}

vector<Vector2> temporalSampling(vector<Vector2> stroke, int num)
{
    double length = 0;
    int cnt = stroke.size();
    vector<Vector2> vec(num);

    rep(i, cnt-1)
        length += dist(stroke[i], stroke[i+1]);
    if (cnt == 1 || length <= eps)
    {
        rep(i, num)
            vec[i] = stroke[0];
        return vec;
    }
    double increment = length / (num - 1);
    Vector2 last = stroke[0];
    double distSoFar = 0;
    int id = 1, vecID = 1;
    vec[0] = stroke[0];

    while (id < cnt)
    {
        double d = dist(last, stroke[id]);
        if (distSoFar + d >= increment)
        {
            double Ratio = (increment - distSoFar) / d;
            last = last + (stroke[id] - last) * Ratio;
            vec[vecID++] = last;
            distSoFar = 0;
        }
        else
        {
            distSoFar += d;
            last = stroke[id++];
        }
    }
    for (int i = vecID; i < num; ++i)
        vec[i] = stroke[cnt - 1];
    return vec;
}

vector<Vector2> normalize(vector<Vector2> stroke)
{
    int num = stroke.size();
    Vector2 center, minV(INF, INF), maxV(-INF, -INF);
    rep(i, num)
        center = center + stroke[i];
    center = center / num;
    vector<Vector2> vec(num);
    rep(i, num)
    {
        vec[i] = stroke[i] - center;
        minV.x = min(minV.x, vec[i].x);
        minV.y = min(minV.y, vec[i].y);
        maxV.x = max(maxV.x, vec[i].x);
        maxV.y = max(maxV.y, vec[i].y);
    }
    double scale = min(maxV.x - minV.x, maxV.y - minV.y);
    rep(i, num)
        vec[i] = vec[i] / scale;
    return vec;
}

enum Formula
{
    Standard = 0,
    DTW = 1,
    DTW_H = 2,
    DTWL = 3,
};

inline double det(const Vector2& a, const Vector2& b, const Vector2& c)
{
    return (b.x-a.x) * (c.y-a.y) - (b.y-a.y) * (c.x-a.x);
}

double pointToSeg(const Vector2& p, const Vector2& a, const Vector2& b)
{
    double cross = (b.x-a.x) * (p.x-a.x) + (b.y-a.y) * (p.y-a.y);
    if (cross <= 0)
        return dist(p, a);
    double segLen = dist(a, b);
    if (cross >= segLen * dist(p, a))
        return dist(p, b);
    return fabs(det(a, b, p)) / segLen;
}

double match(const vector<Vector2>& A, vector<Vector2>& B,
             double dtw[MAXSAMPLE][MAXSAMPLE], Formula formula, double terminate = INF)
{
    if (A.size() != B.size() && formula != DTWL)
        return INF;
    double dis = 0;
    int num = A.size(), w;
    terminate *= num;
    switch(formula)
    {
    case (Standard):
        rep(i, num)
        {
            dis += dist(A[i], B[i]);
            if (dis > terminate)
                return INF;
        }
        break;

    case (DTW):
        //w = max(num * 0.1, 3.0);
        w = 3;
        For(i, num)
        {
            double gap = INF;
            FOR(j, max(1, i - w), min(i + w, num))
            {
                dtw[i][j] = dist(A[i-1], B[j-1]) + min(dtw[i-1][j-1], min(dtw[i][j-1], dtw[i-1][j]));
                gap = min(dtw[i][j], gap);
            }
            if (gap > terminate)
                return INF;
        }
        dis = dtw[num][num];
        break;

    case (DTW_H):
        w = max(num * 0.1, 3.0);
        For(i, num)
        {
            double gap = INF;
            FOR(j, max(1, i - w), min(i + w, num))
            {
                double d = dist(A[i-1], B[j-1]);
                if (i + j < 10)
                    d *= (i+j) * 0.1;
                dtw[i][j] = d + min(dtw[i-1][j-1], min(dtw[i][j-1], dtw[i-1][j]));
                gap = min(dtw[i][j], gap);
            }
            if (gap > terminate)
                return INF;
        }
        dis = dtw[num][num];
        break;

    case (DTWL):
        int len = B.size() - 1;
        For(i, num)
        {
            double gap = INF;
            For(j, min(i, len))
            {
                dtw[i][j] = min(dtw[i-1][j-1], dtw[i-1][j]) + pointToSeg(A[i-1], B[j-1], B[j]);
                gap = min(gap, dtw[i][j]);
            }
            //if (gap > terminate)
                //return INF;
        }
        dis = dtw[num][len];
        break;
    }
    return dis / num;
}


#endif // VECTOR2_H_
