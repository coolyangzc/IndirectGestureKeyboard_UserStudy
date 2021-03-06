#ifndef COMMON_H_
#define COMMON_H_

#include <cmath>
#include <string>
#include <cstdlib>

#define rep(i,n) for (int i=0; i<n; ++i)
#define For(i,n) for (int i=1; i<=n; ++i)
#define FOR(i,a,b) for (int i=a; i<=b; ++i)

const int PHRASES = 96;
const double eps = 1e-6;
const double inf = 1e10;
const int MAXSAMPLE = 200 + 1;
const int LEXICON_SIZE = 10000;

const int USER_NUM = 16;
const std::string id[USER_NUM] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
const std::string user[USER_NUM] = {"DTW/wcj", "DTW/yzc", "DTW/gyz", "DTW/plh", "DTW/yx", "DTW/yyk",
                                    //  1          2          3          4          5         6
                                    "DTW/wrl", "DTW/lzn", "DTW/yzp", "DTW/xwj", "DTW/maye", "DTW/zq",
                                    //  7          8          9         10         11          12
                                    "DTW/zmy", "DTW/zft", "DTW/pxy", "DTW/dzj"};
                                    // 13         14         15         16


int Random(int mo)
{
    return rand() % mo;
}

bool same(const double& x, const double& y)
{
    return (fabs(x-y) < eps);
}

#endif // VECTOR2_H_
