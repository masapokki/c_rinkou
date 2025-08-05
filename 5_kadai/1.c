#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int N;  // 行列のサイズ
int B;  // バンド幅
double *b;  // 右辺ベクトル
double *x;  // 解ベクトル
FILE *fp;

// バンド幅を計算する関数
int get_bandwidth(char* matfile, int n) {
    FILE* fp_band;
    int i, j;
    double tmp;
    int bandwidth = 0;

    if ((fp_band = fopen(matfile, "r")) == NULL) {
        printf("Matrix file open error for bandwidth calculation\n");
        exit(1);
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (fscanf(fp_band, "%lf", &tmp) != 1) {
                printf("Matrix file read error during bandwidth calculation at (%d, %d)\n", i, j);
                fclose(fp_band);
                exit(1);
            }
            if (j >= i && tmp != 0.0) {
                if (j - i + 1 > bandwidth) {
                    bandwidth = j - i + 1;
                }
            }
        }
    }

    fclose(fp_band);
    return bandwidth;
}

void memory_allocate(double*** A, int n, int b_width) {
    int i;
    if((*A = (double**)malloc(n * sizeof(double*))) == NULL){
        printf("No memories are available (a)\n");
        exit(1);
    }

    for(i = 0; i < n; i++){
        if(((*A)[i] = (double*)malloc(b_width * sizeof(double))) == NULL){
            printf("No memories are available (A[%d])\n", i);
            exit(1);
        }
    }

    // bとxのメモリ割り当て
    if((b = (double*)malloc(n * sizeof(double))) == NULL){
        printf("No memories are available (b)\n");
        exit(1);
    }
    if((x = (double*)malloc(n * sizeof(double))) == NULL){
        printf("No memories are available (x)\n");
        exit(1);
    }
}

void read_matrix(double** A, int n, int b_width, char* matfile){
    int i, j;
    double tmp;
    printf("Matrix file: %s\n", matfile);

    if((fp = fopen(matfile, "r")) == NULL){
        printf("Matrix file open error\n");
        exit(1);
    }

    // バンド行列Aを0で初期化
    for(i=0; i<n; i++) {
        for(j=0; j<b_width; j++) {
            A[i][j] = 0.0;
        }
    }

    // 行列Aの読み込み
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            if(fscanf(fp, "%lf", &tmp) == EOF){
                printf("Matrix file read error at position (%d,%d)\n", i, j);
                exit(1);
            }
            if(j >= i){
                if (j - i < b_width) {
                    A[i][j-i] = tmp;
                }
            } else { // j < i
                if (i - j < b_width) {
                    if(tmp != A[j][i-j]){
                        printf("Matrix is not symmetric at (%d, %d)\n", i, j);
                        exit(1);
                    }
                }
            }
        }
    }

    fclose(fp);
}

void read_vector(int n, char* vecfile){
    int i;
    printf("Vector file: %s\n", vecfile);

    if((fp = fopen(vecfile, "r")) == NULL){
        printf("Vector file open error\n");
        exit(1);
    }

    // 右辺ベクトルbの読み込み
    for(i = 0; i < n; i++){
        if(fscanf(fp, "%lf", &b[i]) == EOF){
            printf("Vector file read error\n");
            exit(1);
        }
    }

    fclose(fp);
}

void forward_erase(double** A, int n, int b_width){
    int i, j, k;
    double tmp;
    int k_limit;

    for(i = 0; i < n-1; i++){
        if(A[i][0] == 0.0){
            printf("Division by zero at i=%d\n", i);
            exit(1);
        }
        for(j = i+1; j < i + b_width && j < n; j++){
            tmp = A[i][j-i]/A[i][0];
            b[j] -= tmp * b[i];
            k_limit = i + b_width;
            if (k_limit > n) k_limit = n;
            for(k = j; k < k_limit; k++){
                 if (j-i < b_width && k-i < b_width && k-j < b_width) {
                    A[j][k-j] -= tmp * A[i][k-i];
                }
            }
        }
    }
}

void backward_assignment(double** A, int n, int b_width){
    int i, j;

    for(i = n-1; i >= 0; i--){
        for(j = i+1; j < i + b_width && j < n; j++){
            b[i] -= A[i][j-i] * x[j];
        }
        if(A[i][0] == 0.0){
            printf("Division by zero at i=%d\n", i);
            exit(1);
        }
        x[i] = b[i]/A[i][0];
    }
}

void print_solution(int n){
    int i;
    printf("\nSolution:\n");
    for(i = 0; i < n; i++){
        printf("x[%d] = %f\n", i, x[i]);
    }
}

int count_matrix_size(char* filename) {
    FILE* fp;
    int size = 0;
    char line[1024];

    if((fp = fopen(filename, "r")) == NULL){
        printf("Cannot open file %s for size counting\n", filename);
        exit(1);
    }

    // 空でない行の数を数える
    while(fgets(line, sizeof(line), fp) != NULL) {
        if(strlen(line) > 1) { // 改行文字のみの行は無視
            size++;
        }
    }

    fclose(fp);
    return size;
}

int main(int argc, char *argv[]){
    double **A;
    char *matrix_file = NULL;
    char *vector_file = NULL;
    int opt;

    // コマンドライン引数の解析
    while((opt = getopt(argc, argv, "a:b:")) != -1) {
        switch(opt) {
            case 'a':
                matrix_file = optarg;
                break;
            case 'b':
                vector_file = optarg;
                break;
            default:
                printf("Usage: %s -a matrix_file -b vector_file\n", argv[0]);
                exit(1);
        }
    }

    // 必要な引数が指定されているか確認
    if(matrix_file == NULL || vector_file == NULL) {
        printf("Usage: %s -a matrix_file -b vector_file\n", argv[0]);
        exit(1);
    }

    // 行列のサイズを自動的に判定
    N = count_matrix_size(matrix_file);
    printf("Matrix size: %d x %d\n", N, N);

    // バンド幅を計算
    B = get_bandwidth(matrix_file, N);
    printf("Bandwidth: %d\n", B);

    memory_allocate(&A, N, B);
    read_matrix(A, N, B, matrix_file);
    read_vector(N, vector_file);
    
    printf("\nSolving the system...\n");
    forward_erase(A, N, B);
    backward_assignment(A, N, B);
    print_solution(N);

    // メモリの解放
    for(int i = 0; i < N; i++){
        free(A[i]);
    }
    free(A);
    free(b);
    free(x);

    return 0;
}