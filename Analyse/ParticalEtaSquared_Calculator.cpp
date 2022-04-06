#include <cstdio>

using namespace std;

double df_effect, df_error;
double F, eta2;

int main()
{
    puts("F(a, b) = v");

    while (true)
    {
        scanf("%lf%lf%lf", &df_effect, &df_error, &F);
        eta2 = (F * df_effect) / (F * df_effect + df_error);
        printf("F(%lf, %lf) = %lf, ¦Ç^2_p = %lf\n", df_effect, df_error, F, eta2);
    }
}
