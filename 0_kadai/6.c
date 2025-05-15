#include <stdio.h>
#include <stdlib.h>

int main() {
    int n;
    
    printf("A の次元を入力してください: ");
    scanf("%d", &n);
    
    // 行列 A のメモリ確保
    int **A = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        A[i] = (int*)malloc(n * sizeof(int));
    }
    
    // ベクトル a と結果ベクトルのメモリ確保
    int *a = (int*)malloc(n * sizeof(int));
    int *result = (int*)malloc(n * sizeof(int));
     
    // 行列 A の入力
    printf("行列 A の要素を入力してください:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("A[%d][%d]: ", i+1, j+1);
            scanf("%d", &A[i][j]);
        }
    }
    
    // ベクトル a の入力
    printf("ベクトル a の要素を入力してください:\n");
    for (int i = 0; i < n; i++) {
        printf("a[%d]: ", i+1);
        scanf("%d", &a[i]);
    }
    
    // A*a の計算
    for (int i = 0; i < n; i++) {
        result[i] = 0;
        for (int j = 0; j < n; j++) {
            result[i] += A[i][j] * a[j];
        }
    }
    
    // 結果の表示
    printf("Aa の結果:\n");
    printf("{ ");
    for (int i = 0; i < n; i++) {
        printf("%d", result[i]);
    }
    
    // メモリの解放
    for (int i = 0; i < n; i++) {
        free(A[i]);
    }
    free(A);
    free(a);
    free(result);
    
    return 0;
}