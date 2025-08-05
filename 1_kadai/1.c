#include <stdio.h>
#include <stdlib.h>


int Size_count(char *vector_file) // mainからの呼び出しと printf 文から引数を推定
{
    FILE *fp;
    char c;
    int N = 0;

    printf("Vectror file: %s\n", vector_file); 
    fp = fopen(vector_file, "r"); 
    if (fp == NULL) { 
        printf("Vectror file not found!!\n"); 
        exit(1); 
    }
    
    // fgetcで一文字ずつ読み込む-ファイル終端（EOF）まで繰り返し
    while ((c = fgetc(fp)) != EOF) { 
        if (c == '\n') { 
            N++; 
        }
    }
    fclose(fp); 
    return N;  
}


// 行列A、ベクトルb、ベクトルcのメモリを確保する関数
void Memory_allocate(int N, double ***A, double **b, double **c) 
{
    int i; 

    *A = (double **)malloc(N * sizeof(double *)); 
    if (*A == NULL) { 
        printf("No memories are available (A)\n"); 
        exit(1); 
    }
    for (i = 0; i < N; i++) { 
        (*A)[i] = (double *)malloc(N * sizeof(double)); 
        if ((*A)[i] == NULL) { 
            printf("No memories are available (A[%d])\n", i); 
            exit(1); 
        }
    }

    *b = (double *)malloc(N * sizeof(double)); 
    if (*b == NULL) { 
        printf("No memories are available (b)\n"); 
        exit(1); 
    }

    *c = (double *)malloc(N * sizeof(double)); 
    if (*c == NULL) { 
        printf("No memories are available (c)\n"); 
        exit(1); 
    }
}

void File_read(int N, double **A, double *b, char *mat_file, char *vec_file) {
    FILE *fp;
    int i, j;

    // Matrix file read
    printf("Matrix file: %s\n", mat_file);
    fp = fopen(mat_file, "r");
    if (fp == NULL) {
        printf("Matrix file not found!!\n");
        exit(1);
    }

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            // fscanfを使って行列の要素をdouble型で読み込む
            if(fscanf(fp, "%lf", &A[i][j]) == EOF){
                printf("vector file read error [%d]\n", i);
                exit(1);
            }
        }
    }
    fclose(fp);

    // Vector file read
    fp = fopen(vec_file, "r");

    for (i = 0; i < N; i++) {
        if (fscanf(fp, "%lf", &b[i]) == EOF) { 
            printf("Vector file read error b[%d]\n", i);
            exit(1);
        }
    }
    fclose(fp);
}

// 行列とベクトルの乗算を実行する関数
void Matrix_vector_multiplier(int N, double **A, double *b, double *c) {
    int i, j;

    for (i = 0; i < N; i++) {
        c[i] = 0.0; 
        for (j = 0; j < N; j++) {
            c[i] += A[i][j] * b[j]; 
        }
    }
}

void Print_results(int N, double *c) {
    int i;

    for (i = 0; i < N; i++) {
        printf("c[%d] = %10.3e\n", i, c[i]);
    }
}

int main(int argc, char *argv[]) {
    int N;
    double **A, *b, *c;

    if (argc < 3) {
        printf("Usage:\n");
        printf("program matrix_data_file vector_data_file\n");
        exit(1);
    }

    N = Size_count(argv[2]); 
    printf("Data size (N): %d\n", N);

    Memory_allocate(N, &A, &b, &c);
    File_read(N, A, b, argv[1], argv[2]); 
    Matrix_vector_multiplier(N, A, b, c);
    Print_results(N, c);

    return 0;
}