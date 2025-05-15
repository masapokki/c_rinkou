#include <stdio.h>
#include <math.h>

int main(void)
{
    int m, n;
    int kai = 0, temp = 0;
    
    printf("mの値を入力してください：");
    scanf("%d", &m);
    printf("nの値を入力してください：");
    scanf("%d", &n);

    printf("mの値は%dです。\n", m);
    printf("nの値は%dです。\n", n);

    for(int j = 0; j < m; j++)
    {
        for(int i = 0; i < n; i++){
            temp += m * n;
        }
        kai += temp;
    }
    printf("解は%dです。\n", kai);

    return 0;
}