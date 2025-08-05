#include <stdio.h>
#include <math.h>

int main(void)
{
    int n, kai = 0;
    scanf("%d", &n);

    for (int i = 1; i <= n; i++)
    {
        kai += i * (-1)^(i-1);
    }
    printf("%d\n", kai);
    return 0;
}