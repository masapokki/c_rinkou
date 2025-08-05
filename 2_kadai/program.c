#include <stdio.h>
#include <stdlib.h>
#include <math.h> // fabsを使用する場合 (現在は使用していません)

// 関数のプロトタイプ宣言
void read_data(const char *filename, double ***matrix_ptr, double **vector_ptr, int *n_ptr); // データ読み込み
void forward_elimination(double **A, double *b, int n); // 前進消去
void backward_substitution(double **A, double *b, double *x, int n); // 後退代入
void display_results(double *x, int n); // 結果表示 (簡略版)
void free_memory(double **A, double *b, double *x, int n_val); // メモリ解放

// メイン関数
int main() {
    double **A = NULL;          // 係数行列
    double *b = NULL;           // 右辺ベクトル
    double *x = NULL;           // 解ベクトル
    int n = 0;                  // システムの次元

    const char *filename = "input.txt"; // 入力ファイル名

    // ファイルからデータを読み込む
    printf("ファイルからデータを読み込んでいます: %s\\n", filename);
    read_data(filename, &A, &b, &n);

    if (n == 0) { // read_data はエラー時に n=0 を設定
        fprintf(stderr, "データ読み込みエラーのため終了します。\\n");
        return 1;
    }

    // 解ベクトル用のメモリを割り当てる
    x = (double *)malloc(n * sizeof(double));
    if (x == NULL) {
        fprintf(stderr, "エラー: 解ベクトル x のメモリ割り当てに失敗しました。\\n");
        free_memory(A, b, NULL, n); // Aとbのみ解放
        return 1;
    }

    // 前進消去を実行
    printf("\\n前進消去を実行中...\\n");
    forward_elimination(A, b, n);

    // 後退代入を実行
    printf("\\n後退代入を実行中...\\n");
    backward_substitution(A, b, x, n);

    // 結果を表示
    printf("\\n--- 結果 ---\\n");
    display_results(x, n); // 引数を変更

    // 割り当てられたメモリを解放
    printf("\\nメモリを解放中...\\n");
    free_memory(A, b, x, n);

    printf("プログラムが終了しました。\\n");
    return 0;
}

void read_data(const char *filename, double ***matrix_ptr, double **vector_ptr, int *n_ptr) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("ファイルオープンエラー");
        *n_ptr = 0;
        return;
    }

    // 行数をカウントして n を決定する
    int lines = 0;
    char ch;
    while(!feof(file)) {
        ch = fgetc(file);
        if(ch == '\n') {
            lines++;
        }
    }
    // ファイルの最後の行が改行で終わっていない場合もカウントする
    // ただし、一般的なテキストファイルでは、内容の最後の行は改行で終わる。
    // (行列の行数) + (ベクトルの要素数) = 総行数 と仮定する。
    // ファイル形式が行列用に N 行、ベクトル用に N 行と厳密に決まっている場合、総行数は 2N となる。
    // したがって、n = lines / 2。

    if (lines == 0) {
        fprintf(stderr, "エラー: ファイルが空か、改行がありません。\\n");
        fclose(file);
        *n_ptr = 0;
        return;
    }
    if (lines % 2 != 0) {
        fprintf(stderr, "エラー: ファイルの行数 (%d) は偶数である必要があります (行列 N + ベクトル N)。\\n", lines);
        fclose(file);
        *n_ptr = 0;
        return;
    }
    *n_ptr = lines / 2;
    int n_val = *n_ptr;

    if (n_val <= 0) {
        fprintf(stderr, "エラー: 計算された次元 n (%d) が正ではありません。\\n", n_val);
        fclose(file);
        *n_ptr = 0;
        return;
    }

    // データを読み込むためにファイルポインタを先頭に戻す
    rewind(file);

    //fscanf(file, "%d", n_ptr) != 1 || *n_ptr <= 0) {
    //    fprintf(stderr, "エラー: ファイルに無効な、または欠落した次元があります。\\n");
    //    fclose(file);
    //    *n_ptr = 0;
    //    return;
    //}
    //int n_val = *n_ptr;


    *matrix_ptr = (double **)malloc(n_val * sizeof(double *));
    if (*matrix_ptr == NULL) {
        fprintf(stderr, "エラー: 行列の行のメモリ割り当てに失敗しました。\\n");
        fclose(file);
        *n_ptr = 0;
        return;
    }
    for (int i = 0; i < n_val; i++) {
        (*matrix_ptr)[i] = (double *)malloc(n_val * sizeof(double));
        if ((*matrix_ptr)[i] == NULL) {
            fprintf(stderr, "エラー: 行列の列のメモリ割り当てに失敗しました (行 %d)。\\n", i);
            for (int k = 0; k < i; k++) free((*matrix_ptr)[k]);
            free(*matrix_ptr);
            *matrix_ptr = NULL;
            fclose(file);
            *n_ptr = 0;
            return;
        }
    }

    *vector_ptr = (double *)malloc(n_val * sizeof(double));
    if (*vector_ptr == NULL) {
        fprintf(stderr, "エラー: ベクトル b のメモリ割り当てに失敗しました。\\n");
        for (int i = 0; i < n_val; i++) free((*matrix_ptr)[i]);
        free(*matrix_ptr);
        *matrix_ptr = NULL;
        fclose(file);
        *n_ptr = 0;
        return;
    }

    printf("係数行列 A (%dx%d) を読み込んでいます:\\n", n_val, n_val);
    for (int i = 0; i < n_val; i++) {
        for (int j = 0; j < n_val; j++) {
            if (fscanf(file, "%lf", &(*matrix_ptr)[i][j]) != 1) {
                fprintf(stderr, "行列要素 A[%d][%d] の読み込みエラー。\\n", i, j);
                free_memory(*matrix_ptr, *vector_ptr, NULL, n_val);
                *matrix_ptr = NULL; *vector_ptr = NULL;
                fclose(file);
                *n_ptr = 0;
                return;
            }
            // printf("%8.3f ", (*matrix_ptr)[i][j]); // Optional: print while reading
        }
        // printf("\n"); // Optional: print while reading
    }

    printf("右辺ベクトル b (%dx1) を読み込んでいます:\\n", n_val);
    for (int i = 0; i < n_val; i++) {
        if (fscanf(file, "%lf", &(*vector_ptr)[i]) != 1) {
            fprintf(stderr, "ベクトル要素 b[%d] の読み込みエラー。\\n", i);
            free_memory(*matrix_ptr, *vector_ptr, NULL, n_val);
             *matrix_ptr = NULL; *vector_ptr = NULL;
            fclose(file);
            *n_ptr = 0;
            return;
        }
        // printf("%8.3f\n", (*vector_ptr)[i]); // Optional: print while reading
    }
    printf("データ読み込み完了。\\n");
    fclose(file);
}

void forward_elimination(double **A, double *b, int n) {
    for (int k = 0; k < n - 1; k++) {
        if (A[k][k] == 0.0) { // ピボット要素がゼロかどうかの基本的なチェック
            fprintf(stderr, "警告: ピボット要素 A[%d][%d] がゼロです。ピボット選択は実装されていません。解が正しくないか、失敗する可能性があります。\\n", k, k);
            // ピボット選択は実装されていないため、特異行列や悪条件の行列に対してはリスクを伴うか、失敗が予想される
        }

        for (int i = k + 1; i < n; i++) {
            if (A[k][k] == 0.0) { // ゼロ除算を回避
                fprintf(stderr, "エラー: ピボット A[%d][%d] がゼロのため、A[%d][%d] を消去できません。\\n",i,k,k,k);
                continue; // ピボットがゼロの場合、この消去をスキップする (結果は誤りになる)
            }
            double factor = A[i][k] / A[k][k];
            for (int j = k; j < n; j++) {
                A[i][j] = A[i][j] - factor * A[k][j];
            }
            b[i] = b[i] - factor * b[k];
        }
    }
    printf("前進消去完了。\\n");
    printf("結果として得られた上三角行列 A と修正されたベクトル b:\\n");
    printf("行列 A (上三角):\\n");
    for(int i=0; i<n; i++) {
        for(int j=0; j<n; j++) {
            printf("%10.3f ", A[i][j]);
        }
        printf("\n");
    }
    printf("ベクトル b (修正済み):\\n");
    for(int i=0; i<n; i++) {
        printf("%10.3f\n", b[i]);
    }
}

void backward_substitution(double **A, double *b, double *x, int n) {
    if (A[n-1][n-1] == 0.0) {
        fprintf(stderr, "エラー: A[%d][%d] がゼロです。後退代入を実行できません (最後の要素)。行列が特異である可能性があります。\\n", n-1, n-1);
        for(int i=0; i<n; ++i) x[i] = NAN; // 非数 (Not-a-Number)
        return;
    }
    x[n-1] = b[n-1] / A[n-1][n-1];

    for (int i = n - 2; i >= 0; i--) {
        double sum = 0.0;
        for (int j = i + 1; j < n; j++) {
            sum += A[i][j] * x[j];
        }
        if (A[i][i] == 0.0) {
            fprintf(stderr, "エラー: 後退代入中に A[%d][%d] がゼロです。行列が特異である可能性があります。\\n", i, i);
            for(int k=0; k<n; ++k) x[k] = NAN; // 解を無効としてマーク
            return;
        }
        x[i] = (b[i] - sum) / A[i][i];
    }
    printf("後退代入完了。\\n");
}

void display_results(double *x, int n) { // 引数から initial_A, initial_b を削除
    printf("\\n解ベクトル x:\\n");
    for (int i = 0; i < n; i++) {
        printf("x[%d] = %10.3f\n", i, x[i]);
    }
}

void free_memory(double **A, double *b, double *x, int n_val) { // メモリ解放
    if (A != NULL) {
        for (int i = 0; i < n_val; i++) {
            if (A[i] != NULL) {
                free(A[i]);
            }
        }
        free(A);
    }
    if (b != NULL) {
        free(b);
    }
    if (x != NULL) {
        free(x);
    }
}