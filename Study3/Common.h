#ifndef COMMON_H_
#define COMMON_H_

#include <cmath>
#include <string>
#include <cstdlib>

#define rep(i,n) for (int i=0; i<n; ++i)
#define For(i,n) for (int i=1; i<=n; ++i)
#define FOR(i,a,b) for (int i=a; i<=b; ++i)
#define sqr(x) ((x)*(x))
#define mk make_pair

typedef long long ll;

const int PHRASES = 60;
const double eps = 1e-6;
const double INF = 1e10;
const int MAXSAMPLE = 200 + 1;
const int LEXICON_SIZE = 10000;

const int USER_NUM = 18;
const std::string id[USER_NUM] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18"};
const std::string user[USER_NUM] =
{"yzc", "maye", "xwj", "yym", "yzp", "cool", "wjh", "yyk", "wrl", "gyz", "yezp", "lyq", "dyp", "plh", "wcj", "ry", "zc", "zmy"};
//  1       2      3      4      5       6      7      8      9     10      11     12     13     14     15    16    17     18

int Random(int mo)
{
    return rand() % mo;
}

bool same(const double& x, const double& y)
{
    return (fabs(x-y) < eps);
}

#endif // VECTOR2_H_
