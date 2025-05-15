#include <stdio.h>

int main(void)
{
    int n;
    printf("nの値を入力してください：");
    scanf("%d", &n);
    printf("%dが入力されました。\n", n);

    if(n == 2)
    {
        printf("%dは素数です。\n", n);
        return 0;
    }
    if(n == 1 || n % 2 == 0)
    {
        printf("%dは素数ではありません。\n", n);
        return 0;
    }else{
        for(int i = 3; i < n-1; i+=2)
        {
            if(n % i == 0){
                printf("%dは素数ではありません。\n", n);
                return 0;
            }
        }
    }
    printf("%dは素数です。\n", n);
    return 0;
}

