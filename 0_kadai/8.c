#include <stdio.h>
#include <stdlib.h>

int main() {
    int n, m;
    
    printf("ベクトル a の次元と行列 A の行数を入力してください (n): ");
    scanf("%d", &n);
    
    printf("行列 B の列数を入力してください (m): ");
    scanf("%d", &m);
    
    // ベクトル a のメモリ確保
    int *a = (int*)malloc(n * sizeof(int));
    
    // 行列 A のメモリ確保 (n x n)
    int **A = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        A[i] = (int*)malloc(n * sizeof(int));
    }
    
    // 行列 B のメモリ確保 (n x m)
    int **B = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        B[i] = (int*)malloc(m * sizeof(int));
    }
    
    // 中間結果 AB のメモリ確保 (n x m)
    int **AB = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        AB[i] = (int*)malloc(m * sizeof(int));
    }
    
    // 最終結果 a^T AB のメモリ確保 (1 x m)
    int *result = (int*)malloc(m * sizeof(int));
    
    // メモリ確保の確認
    if (a == NULL || A == NULL || B == NULL || AB == NULL || result == NULL) {
        printf("メモリの確保に失敗しました。\n");
        return 1;
    }
    
    // ベクトル a の入力
    printf("ベクトル a の要素を入力してください:\n");
    for (int i = 0; i < n; i++) {
        printf("a[%d]: ", i+1);
        scanf("%d", &a[i]);
    }
    
    // 行列 A の入力
    printf("行列 A の要素を入力してください(%d x %d):\n", n, n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("A[%d][%d]: ", i+1, j+1);
            scanf("%d", &A[i][j]);
        }
    }
    
    // 行列 B の入力
    printf("行列 B の要素を入力してください(%d x %d):\n", n, m);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("B[%d][%d]: ", i+1, j+1);
            scanf("%d", &B[i][j]);
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            AB[i][j] = 0.0;
            for (int k = 0; k < n; k++) {
                AB[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    for (int j = 0; j < m; j++) {
        result[j] = 0.0;
        for (int i = 0; i < n; i++) {
            result[j] += a[i] * AB[i][j];
        }
    }
    
    // 結果の表示
    printf("a^T AB の結果:\n");
    printf("[ ");
    for (int j = 0; j < m; j++) {
        printf("%d", result[j]);
        if (j < m-1) printf(", ");
    }
    printf(" ]\n");
    
    // メモリの解放
    free(a);
    for (int i = 0; i < n; i++) {
        free(A[i]);
        free(B[i]);
        free(AB[i]);
    }
    free(A);
    free(B);
    free(AB);
    free(result);
    
    return 0;
}