#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 4

void input_matrix(double **a, char c, FILE *fin, FILE *fout);
void input_vector(double *b, char c, FILE *fin, FILE *fout);
double **dmatrix(int nr1, int nr2, int nl1, int nl2);
void free_dmatrix(double **a, int nr1, int nr2, int nl1, int nl2);
void *dvector(int i, int j);
void free_dvector(void *a, int i);
double *gauss(double **a, double *b);

int main(void)
{
    FILE *fin, *fout;
    double **a, *b;
    int i;

    a = dmatrix(1, N, 1, N);
    b = dvector(1, N);

    if ((fin = fopen("input.dat", "r")) == NULL)
    {
        fprintf(stderr, "Error opening input file\n");
        exit(1);
    }

    if ((fout = fopen("output.dat", "w")) == NULL)
    {
        fprintf(stderr, "Error opening output file\n");
        exit(1);
    }

    input_matrix(a, 'A', fin, fout);
    input_vector(b, 'B', fin, fout);

    b = gauss(a, b);

    fprintf(fout, "Ax = bの解は次の通りです\n");
    for (i = 1; i <= N; i++)
    {
        fprintf(fout, "x[%d] = %f\n", i, b[i]);
    }

    fclose(fin);
    fclose(fout);

    free_dmatrix(a, 1, N, 1, N);
    free_dvector(b, 1);

    return 0;
}

// 部分ピボット選択付きのガウスの消去法
double *gauss(double **a, double *b)
{
    int i, j, k, ip;
    double alpha, tmp;
    double amax, eps = pow(2.0, -50.0);

    for (k = 1; k <= N - 1; k++)
    {
        // ピボット選択
        amax = fabs(a[k][k]);
        ip = k;
        for (i = k + 1; i <= N; i++)
        {
            if (fabs(a[i][k]) > amax){
                amax = fabs(a[i][k]);
                ip = i;
            }
        }

        // 正則性の判定
        if (amax < eps)
        {
            fprintf(stderr, "係数行列が正則ではありません\n");
            exit(1);
        }
        // 行交換
        if (ip != k)
        {
            for (j = k; j <= N; j++)
            {
                tmp = a[k][j];
                a[k][j] = a[ip][j];
                a[ip][j] = tmp;
            }
            tmp = b[k];
            b[k] = b[ip];
            b[ip] = tmp;
        }

        // 前進消去
        for (i = k + 1; i <= N; i++)
        {
            alpha = a[i][k] / a[k][k];
            for (j = k + 1; j <= N; j++)
            {
                a[i][j] -= alpha * a[k][j];
            }
            b[i] -= alpha * b[k];
        }
    }

    // 後退代入
    b[N] = b[N] / a[N][N];
    for (k = N - 1; k >= 1; k--)
    {
        tmp = b[k];
        for (j = k + 1; j <= N; j++)
        {
            tmp -= a[k][j] * b[j];
        }
        b[k] = tmp / a[k][k];
    }

    return b;
}


