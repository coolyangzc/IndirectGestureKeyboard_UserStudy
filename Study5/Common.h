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

const int PHRASES = 80;
const int MAXSAMPLE = 100;
const double eps = 1e-6;
const double INF = 1e10;
const int LEXICON_SIZE = 10000;

const int USER_NUM = 24;
const std::string user[USER_NUM + 1] =
{"Null", "yzc", "zc", "gyz", "yyk", "yezp", "lxh", "wxy", "guyz", "lzx", "maye", "cxs", "lzn",
//   0      1     2      3      4       5      6      7       8      9      10     11     12
 "lyq", "hhs", "xxh", "swn", "wrl", "fsy", "rj", "ljh", "yk", "sk", "lgh", "shuyi"};
// 13     14     15     16     17     18    19     20    21    22     23       24

int Random(int mo)
{
    return rand() % mo;
}

bool same(const double& x, const double& y)
{
    return (fabs(x-y) < eps);
}

#endif // COMMON_H_
