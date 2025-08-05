#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 10                // N次元方程式
#define EPS pow(10.0, -8.0) // epsilon の設定
#define KMAX 100            // 最大反復回数

/* 関数のプロトタイプ宣言 */

// 行列の入力
void input_matrix(double **a, char c, FILE *fin, FILE *fout);

// ベクトルの入力
void input_vector(double *b, char c, FILE *fin, FILE *fout);

// 行列の領域確保
double **dmatrix(int nr1, int nr2, int nl1, int nl2);

// 行列の領域解放
void free_dmatrix(double **a, int nr1, int nr2, int nl1, int nl2);

// ベクトル領域の確保
double *dvector(int i, int j);

// 領域の解放
void free_dvector(double *a, int i);

// 1ノルムの計算 a[m...n]
double vector_norm1(double *a, int m, int n);

// ベクトル a[m...n] と b[m...n] の内積を計算する
double inner_product(int m, int n, double *a, double *b);

// 行列 a[1...N][1...N] とベクトル b[1...N] との積 c<-Ab
void matrix_vector_product(double **a, double *b, double *c);

// 共役勾配法(CG法)
double *cg(double **a, double *b, double *x);


/* main 関数 */
int main(void)
{
  FILE *fin, *fout;
  double **a, *b, *x;
  int i;

  /* 行列およびベクトルの領域確保 */
  a = dmatrix(1, N, 1, N); /* 行列 a[1...N][1...N] */
  b = dvector(1, N);      /* b[1...N] */
  x = dvector(1, N);      /* x[1...N] */

  /* ファイルのオープン */
  if ( (fin = fopen("input_matrix.txt", "r")) == NULL )
  {
    printf("ファイルが見つかりません : input_matrix.txt \n");
    exit(1);
  }
  if ( (fout = fopen("output.dat", "w")) == NULL )
  {
    printf("ファイルが作成できません : output.dat \n");
    exit(1);
  }

  input_matrix( a, 'A', fin, fout ); /* 行列Aの入力 */
  
  fclose(fin); /* 行列ファイルを閉じる */
  
  /* ベクトルファイルのオープン */
  if ( (fin = fopen("input_vector.txt", "r")) == NULL )
  {
    printf("ファイルが見つかりません : input_vector.txt \n");
    exit(1);
  }
  
  input_vector( b, 'b', fin, fout ); /* ベクトルbの入力 */
  
  /* 初期ベクトルx0の設定（零ベクトル） */
  for( i = 1; i <= N; i++ )
  {
    x[i] = 0.0;
  }
  
  x = cg( a, b, x );                  /* 共役勾配法(CG法) */

  /* 結果の出力 */
  fprintf(fout, "Ax=b の解は次の通りです\n");
  for( i = 1; i <= N; i++ )
  {
    fprintf(fout, "x[%d] = %f\n", i, x[i]);
  }

  fclose(fin); fclose(fout); /* ファイルのクローズ */
  
  /* 領域の解放 */
  free_dmatrix( a, 1, N, 1, N ); free_dvector( b, 1 ); free_dvector( x, 1 );
  return 0;
}

/* 共役勾配法 (CG法) */
double *cg(double **a, double *b, double *x)
{
  double eps, *r, *p, *tmp, alpha, beta, work;
  double rho, rho_new; // rとrの内積を格納する変数
  int i, k=0;

  r = dvector(1,N);
  p = dvector(1,N);
  tmp = dvector(1,N);
  
  // 初期化: r_0 = p_0 = b - A*x_0
  matrix_vector_product(a, x, tmp);
  for ( i = 1; i <= N; i++ ) {
    p[i] = b[i] - tmp[i];
    r[i] = p[i];
  }

  do {
    k++;

    // rho = (r_k)^T * r_k を計算
    rho = inner_product( 1, N, r, r );

    // alpha の計算 (式 6.38 a)
    // alpha = (r_k^T * r_k) / (p_k^T * A * p_k)
    matrix_vector_product( a, p, tmp );     // tmp <- A * p_k
    work = inner_product( 1, N, p, tmp ); // work <- p_k^T * A * p_k
    alpha = rho / work;

    // 解 x と 残差 r の更新
    // x_{k+1} = x_k + alpha * p_k
    // r_{k+1} = r_k - alpha * A * p_k
    for( i = 1; i <= N; i++) x[i] = x[i] + alpha*p[i];
    for( i = 1; i <= N; i++) r[i] = r[i] - alpha*tmp[i];
    
    // 収束判定
    eps = vector_norm1(r, 1, N);
    if ( eps < EPS ) goto OUTPUT;

    // beta の計算 (式 6.38 b)
    // beta = (r_{k+1}^T * r_{k+1}) / (r_k^T * r_k)
    rho_new = inner_product( 1, N, r, r ); // r は更新済みなので r_{k+1}
    beta = rho_new / rho; // rho は更新前の r_k の内積

    // 探索方向 p の更新
    // p_{k+1} = r_{k+1} + beta * p_k
    for ( i = 1; i <= N; i++) p[i] = r[i] + beta*p[i];

  } while( k < KMAX );

OUTPUT:
  // 後処理 (変更なし)
  free_dvector(r, 1); free_dvector(p, 1); free_dvector(tmp, 1);
  if ( k == KMAX ) {
    printf("答えが見つかりませんでした\n");
    exit(1);
  } else {
    printf("反復回数は%d回です\n", k);
    return x;
  }
}


/* 行列 a[1...N][1...N] とベクトル b[1...N] との積 c<-Ab */
void matrix_vector_product(double **a, double *b, double *c)
{
  double wk;
  int i, j;

  for ( i = 1; i <= N; i++ )
  {
    wk = 0.0;
    for ( j = 1; j <= N; j++ )
    {
      wk += a[i][j] * b[j];
    }
    c[i] = wk;
  }
}

/* ベクトル領域の確保 */
double *dvector(int i, int j) {
    double *p = (double *)malloc(sizeof(double) * (j - i + 2));
    if (p == NULL) {
        fprintf(stderr, "dvector: メモリ確保に失敗しました。\n");
        exit(1);
    }
    return p - i + 1;
}

/* ベクトル領域の解放 */
void free_dvector(double *a, int i) {
    free(a + i - 1);
}

/* 行列領域の確保 */
double **dmatrix(int nr1, int nr2, int nl1, int nl2) {
    int i;
    double **p = (double **)malloc(sizeof(double *) * (nr2 - nr1 + 2));
    if (p == NULL) {
        fprintf(stderr, "dmatrix: メモリ確保に失敗しました。\n");
        exit(1);
    }
    p = p - nr1 + 1;
    
    for (i = nr1; i <= nr2; i++) {
        p[i] = (double *)malloc(sizeof(double) * (nl2 - nl1 + 2));
        if (p[i] == NULL) {
            fprintf(stderr, "dmatrix: メモリ確保に失敗しました。\n");
            exit(1);
        }
        p[i] = p[i] - nl1 + 1;
    }
    return p;
}

/* 行列領域の解放 */
void free_dmatrix(double **a, int nr1, int nr2, int nl1, int nl2) {
    int i;
    for (i = nr1; i <= nr2; i++) {
        free(a[i] + nl1 - 1);
    }
    free(a + nr1 - 1);
}

/* 行列の入力 */
void input_matrix(double **a, char c, FILE *fin, FILE *fout) {
    int i, j;
    fprintf(fout, "行列%cを入力します\n", c);
    for (i = 1; i <= N; i++) {
        for (j = 1; j <= N; j++) {
            if (fscanf(fin, "%lf", &a[i][j]) != 1) {
                fprintf(stderr, "行列の読み込みエラーです。\n");
                exit(1);
            }
        }
    }
    fprintf(fout, "行列%cの入力が完了しました\n", c);
}

/* ベクトルの入力 */
void input_vector(double *b, char c, FILE *fin, FILE *fout) {
    int i;
    fprintf(fout, "ベクトル%cを入力します\n", c);
    for (i = 1; i <= N; i++) {
        if (fscanf(fin, "%lf", &b[i]) != 1) {
            fprintf(stderr, "ベクトルの読み込みエラーです。\n");
            exit(1);
        }
    }
    fprintf(fout, "ベクトル%cの入力が完了しました\n", c);
}

/* 1ノルムの計算 */
double vector_norm1(double *a, int m, int n) {
    double norm = 0.0;
    int i;
    for (i = m; i <= n; i++) {
        norm += fabs(a[i]);
    }
    return norm;
}

/* ベクトルの内積計算 */
double inner_product(int m, int n, double *a, double *b) {
    double product = 0.0;
    int i;
    for (i = m; i <= n; i++) {
        product += a[i] * b[i];
    }
    return product;
}