#include <cstdio>

using namespace std;

int df_effect, df_error;
double F, eta2;

int main()
{
    puts("F(a, b) = v");

    while (true)
    {
        scanf("%d%d%lf", &df_effect, &df_error, &F);
        eta2 = (F * df_effect) / (F * df_effect + df_error);
        printf("F(%d, %d) = %lf, ¦Ç^2_p = %lf\n", df_effect, df_error, F, eta2);
    }
}
