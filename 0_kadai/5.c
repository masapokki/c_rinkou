#include <stdio.h>
#include <stdlib.h>

int main() {
    int n;
    
    printf("ベクトルの次元を入力してください: ");
    scanf("%d", &n);
    
    // ベクトル a と b のメモリ確保
    int *a = (int   *)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    

    
    // ベクトル a の入力
    printf("ベクトル a の要素を入力してください:\n");
    for (int i = 0; i < n; i++) {
        printf("a[%d]: ", i+1);
        scanf("%d", &a[i]);
    }
    
    // ベクトル b の入力
    printf("ベクトル b の要素を入力してください:\n");
    for (int i = 0; i < n; i++) {
        printf("b[%d]: ", i+1);
        scanf("%d", &b[i]);
    }
    
    // a・b の計算 (内積)
    int dot_product = 0;
    for (int i = 0; i < n; i++) {
        dot_product += a[i] * b[i];
    }
    
    // 結果の表示
    printf("a・b の内積: %d\n", dot_product);
    
    // メモリの解放
    free(a);
    free(b);
    
    return 0;
}