#include <stdio.h>
#include <stdlib.h>

int main() {
    int n, m;
    
    printf("nを入力してください: ");
    scanf("%d", &n);
    
    printf("mを入力してください: ");
    scanf("%d", &m);
    
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
    
    // 結果行列 AB のメモリ確保 (n x m)
    int **AB = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        AB[i] = (int*)malloc(m * sizeof(int));
    }
    
    // 行列 A の入力
    printf("行列 A の要素を入力してください（%d x %d）:\n", n, n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("A[%d][%d]: ", i+1, j+1);
            scanf("%d", &A[i][j]);
        }
    }
    
    // 行列 B の入力
    printf("行列 B の要素を入力してください（%d x %d）:\n", n, m);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("B[%d][%d]: ", i+1, j+1);
            scanf("%d", &B[i][j]);
        }
    }
    
    // AB の計算（行列積）
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            AB[i][j] = 0;
            for (int k = 0; k < n; k++) {
                AB[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    // 結果の表示
    printf("AB の結果:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%d\t", AB[i][j]);
        }
        printf("\n");
    }
    
    // メモリの解放
    for (int i = 0; i < n; i++) {
        free(A[i]);
        free(B[i]);
        free(AB[i]);
    }
    
    free(A);
    free(B);
    free(AB);
    
    return 0;
}