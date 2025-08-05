#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// 対称行列の上三角部分のみのためのデータ構造と関数
double *symmetric_matrix(int n);
void free_symmetric_matrix(double *a);
double *dvector(int i, int j);
void free_dvector(double *a, int i);
int count_matrix_size(FILE *fin);
void input_symmetric_matrix(double *a, int n, FILE *fin, FILE *fout);
void input_vector(double *b, int n, FILE *fin, FILE *fout);
double *gauss_symmetric(double *a, double *b, int n);
int get_upper_index(int i, int j, int n);

int main(int argc, char *argv[])
{
    FILE *fin_matrix, *fin_vector, *fout;
    double *a, *b;
    int i, n;
    char *matrix_file = "input_matrix.txt";
    char *vector_file = "input_vector.txt";
    char *output_file = "output.dat";

    // コマンドライン引数の処理
    if (argc > 1) {
        matrix_file = argv[1];
    }
    if (argc > 2) {
        vector_file = argv[2];
    }

    // 行列ファイルを開いて行数を判定
    if ((fin_matrix = fopen(matrix_file, "r")) == NULL)
    {
        fprintf(stderr, "Error opening matrix file: %s\n", matrix_file);
        exit(1);
    }

    // 行列のサイズを判定
    n = count_matrix_size(fin_matrix);
    fclose(fin_matrix);

    // ベクトルファイルを開く
    if ((fin_vector = fopen(vector_file, "r")) == NULL)
    {
        fprintf(stderr, "Error opening vector file: %s\n", vector_file);
        exit(1);
    }

    if ((fout = fopen(output_file, "w")) == NULL)
    {
        fprintf(stderr, "Error opening output file: %s\n", output_file);
        exit(1);
    }

    fprintf(fout, "行列のサイズ: %d x %d\n", n, n);
    printf("行列のサイズ: %d x %d\n", n, n);
    fprintf(fout, "使用メモリ: %d個の要素（上三角部分のみ）\n", n*(n+1)/2);
    printf("使用メモリ: %d個の要素（上三角部分のみ）\n", n*(n+1)/2);

    a = symmetric_matrix(n);
    b = dvector(1, n);

    // 行列ファイルを再度開いて読み込み
    if ((fin_matrix = fopen(matrix_file, "r")) == NULL)
    {
        fprintf(stderr, "Error reopening matrix file: %s\n", matrix_file);
        exit(1);
    }

    input_symmetric_matrix(a, n, fin_matrix, fout);
    input_vector(b, n, fin_vector, fout);

    b = gauss_symmetric(a, b, n);

    fprintf(fout, "Ax = bの解は次の通りです\n");
    printf("Ax = bの解は次の通りです\n");
    for (i = 1; i <= n; i++)
    {
        fprintf(fout, "x[%d] = %f\n", i, b[i]);
        printf("x[%d] = %f\n", i, b[i]);
    }

    fclose(fin_matrix);
    fclose(fin_vector);
    fclose(fout);

    free_symmetric_matrix(a);
    free_dvector(b, 1);

    return 0;
}

// 行列ファイルから行数を数える
int count_matrix_size(FILE *fin)
{
    int n = 0;
    char line[1024];
    
    while (fgets(line, sizeof(line), fin) != NULL) {
        // 空行でない場合にカウント
        if (line[0] != '\n' && line[0] != '\0') {
            n++;
        }
    }
    
    return n;
}

// 上三角行列のインデックス計算（i,j → 1次元配列のインデックス）
int get_upper_index(int i, int j, int n)
{
    if (i <= j) {
        return (i - 1) * n - (i - 1) * (i - 2) / 2 + (j - i);
    } else {
        // 対称性を利用してj,iのインデックスを返す
        return (j - 1) * n - (j - 1) * (j - 2) / 2 + (i - j);
    }
}

// 対称行列の上三角部分のメモリ割り当て（n(n+1)/2個のみ）
double *symmetric_matrix(int n)
{
    double *a;
    int size = n * (n + 1) / 2;
    
    a = (double *)malloc(size * sizeof(double));
    if (a == NULL) {
        fprintf(stderr, "メモリ割り当てエラー\n");
        exit(1);
    }
    return a;
}

// 対称行列のメモリ解放
void free_symmetric_matrix(double *a)
{
    free(a);
}

// ベクトルのメモリ割り当て（1-indexedベース）
double *dvector(int i, int j)
{
    double *a;
    
    a = (double *)malloc((j - i + 2) * sizeof(double));
    if (a == NULL) {
        fprintf(stderr, "メモリ割り当てエラー\n");
        exit(1);
    }
    return a - i + 1;  // 1-indexedアクセスのための調整
}

// ベクトルのメモリ解放
void free_dvector(double *a, int i)
{
    free(a + i - 1);
}

// 対称行列の入力（上三角部分のみを読み取り）
void input_symmetric_matrix(double *a, int n, FILE *fin, FILE *fout)
{
    int i, j, index;
    double value;
    
    fprintf(fout, "対称行列A（上三角部分）の入力:\n");
    printf("対称行列A（上三角部分）の入力:\n");
    
    for (i = 1; i <= n; i++) {
        for (j = i; j <= n; j++) {
            if (fscanf(fin, "%lf", &value) != 1) {
                fprintf(stderr, "行列要素の読み込みエラー\n");
                exit(1);
            }
            index = get_upper_index(i, j, n);
            a[index] = value;
            fprintf(fout, "A[%d][%d] = %f (インデックス: %d)\n", i, j, value, index);
            printf("A[%d][%d] = %f (インデックス: %d)\n", i, j, value, index);
        }
    }
}

// ベクトルの入力
void input_vector(double *b, int n, FILE *fin, FILE *fout)
{
    int i;
    
    fprintf(fout, "ベクトルbの入力:\n");
    printf("ベクトルbの入力:\n");
    for (i = 1; i <= n; i++) {
        if (fscanf(fin, "%lf", &b[i]) != 1) {
            fprintf(stderr, "ベクトル要素の読み込みエラー\n");
            exit(1);
        }
        fprintf(fout, "b[%d] = %f\n", i, b[i]);
        printf("b[%d] = %f\n", i, b[i]);
    }
}

// 対称行列用のガウスの消去法（部分ピボット選択なし）
double *gauss_symmetric(double *a, double *b, int n)
{
    int i, j, k;
    int idx_ij, idx_ik, idx_kj, idx_kk;
    double alpha, tmp;
    double eps = pow(2.0, -50.0);

    // 前進消去
    for (k = 1; k <= n - 1; k++)
    {
        // 対角要素のインデックス計算 (k,k)
        idx_kk = get_upper_index(k, k, n);
        
        // 正則性の判定
        if (fabs(a[idx_kk]) < eps)
        {
            fprintf(stderr, "係数行列が正則ではありません\n");
            exit(1);
        }

        // 前進消去
        for (i = k + 1; i <= n; i++)
        {
            // (i,k)要素のインデックス計算（対称性により(k,i)と同じ）
            idx_ik = get_upper_index(i, k, n);
            
            alpha = a[idx_ik] / a[idx_kk];
            
            for (j = k + 1; j <= n; j++)
            {
                // (i,j)要素のインデックス計算
                idx_ij = get_upper_index(i, j, n);
                // (k,j)要素のインデックス計算
                idx_kj = get_upper_index(k, j, n);
                
                a[idx_ij] -= alpha * a[idx_kj];
            }
            b[i] -= alpha * b[k];
        }
    }

    // 後退代入
    idx_kk = get_upper_index(n, n, n);  // (n,n)要素
    b[n] = b[n] / a[idx_kk];
    
    for (k = n - 1; k >= 1; k--)
    {
        tmp = b[k];
        for (j = k + 1; j <= n; j++)
        {
            // (k,j)要素のインデックス計算
            idx_kj = get_upper_index(k, j, n);
            tmp -= a[idx_kj] * b[j];
        }
        // (k,k)要素のインデックス計算
        idx_kk = get_upper_index(k, k, n);
        b[k] = tmp / a[idx_kk];
    }

    return b;
}


