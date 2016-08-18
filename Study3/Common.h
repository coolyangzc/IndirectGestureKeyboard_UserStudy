#ifndef COMMON_H_
#define COMMON_H_

#include <cmath>
#include <string>
#include <cstdlib>

#define rep(i,n) for (int i=0; i<n; ++i)
#define For(i,n) for (int i=1; i<=n; ++i)
#define FOR(i,a,b) for (int i=a; i<=b; ++i)

const int PHRASES = 60;
const double eps = 1e-6;
const double inf = 1e10;
const int MAXSAMPLE = 300 + 1;
const int LEXICON_SIZE = 1000;

int Random(int mo)
{
    return rand() % mo;
}

bool same(const double& x, const double& y)
{
    return (fabs(x-y) < eps);
}

#endif // VECTOR2_H_
