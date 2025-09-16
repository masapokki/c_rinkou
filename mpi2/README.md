# MPI 並列行列ベクトル積プログラム解説

このリポジトリには、MPI (Message Passing Interface) を使用して、行列とベクトルの積を並列に計算するC言語プログラムが含まれています。

## 概要

このプログラムは、`rank 0` のプロセスがファイルから行列とベクトルデータを読み込み、他のプロセスに行列の行データをサイクリック（巡回的）に分配して、各プロセスが並列に内積計算を実行します。最後に、`rank 0` が各プロセスから計算結果を集約し、最終的な結果ベクトルを出力します。

通信には、通信と計算のオーバーラップを可能にするノンブロッキング関数 (`MPI_Isend`, `MPI_Irecv`) を使用しており、効率的な並列処理を目指しています。

## 主な特徴

- **ファイルからの自動サイズ認識**: プログラム実行時に、行列とベクトルのサイズをファイルの内容から自動的に読み取ります。
- **コマンドライン引数からのファイル指定**: 計算対象となる行列とベクトルのデータファイルを、コマンドライン引数で柔軟に指定できます。
- **ノンブロッキング通信**: MPIのノンブロッキング通信を利用して、プロセスの待機時間を削減し、パフォーマンスの向上を図っています。
- **サイクリックなデータ分配**: 行列の各行を各プロセスに巡回的に割り当てることで、計算負荷の均等化を図ります。

## ビルドと実行方法

### 1. コンパイル
`mpicc` コンパイラを使用してプログラムをビルドします。

```bash
mpicc mpi2/1.c -o mpi2/matrix_vector_mult
```

### 2. 実行
`mpiexec` または `mpirun` を使用してプログラムを実行します。コマンドライン引数として、行列データファイルとベクトルデータファイルを指定してください。

```bash
# 例: 4プロセスで実行する場合
mpiexec -n 4 ./mpi2/matrix_vector_mult <行列ファイルへのパス> <ベクトルファイルへのパス>

# 具体例
mpiexec -n 4 ./mpi2/matrix_vector_mult mpi2/matrix.txt mpi2/vector.txt
```

---

## プログラムの詳細な解説

### `main` 関数の処理フロー

#### 1.  MPIの初期化
各プロセスはMPI環境を開始し、自身のランク番号と全体のプロセス数を取得します。

- **対応するコード (`1.c`)**
  ```c
  int main(int argc, char **argv) {
      MPI_Init(&argc, &argv);
  
      int world_rank, world_size;
      MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
      MPI_Comm_size(MPI_COMM_WORLD, &world_size);
      // ...
  }
  ```

#### 2.  データ読み込みと準備 (`rank 0` のみ)
`rank 0` がファイル読み込み、メモリ確保など、計算の準備をすべて行います。

- **対応するコード (`1.c`)**
  ```c
  if (world_rank == 0) {
      // コマンドライン引数の数をチェック
      if (argc < 3) {
          fprintf(stderr, "使用法: %s <matrix_file> <vector_file>\n", argv[0]);
          MPI_Abort(MPI_COMM_WORLD, 1);
      }
      char* matrix_filename = argv[1];
      char* vector_filename = argv[2];

      // ファイルから次元を自動取得
      get_matrix_dimensions(matrix_filename, &N, &M);
      int vec_dim;
      get_vector_size(vector_filename, &vec_dim);

      // 行列とベクトルの次元が適合するかチェック
      if (M != vec_dim) {
           fprintf(stderr, "エラー: 行列とベクトルの次元が非互換です...\n");
           MPI_Abort(MPI_COMM_WORLD, 1);
      }

      // メモリ確保とファイル読み込み
      matrix = (double *)malloc(N * M * sizeof(double));
      // ... fscanfによる読み込みループ ...
      vector = (double *)malloc(M * sizeof(double));
      // ... fscanfによる読み込みループ ...
  }
  ```

#### 3.  データ分配
`rank 0` が準備したデータを、他の全プロセスに送信します。

- **対応するコード (`1.c`)**
  - **計算情報 (次元、ベクトル) のブロードキャスト**
    ```c
    // 全プロセスに次元をブロードキャスト
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // ... rank != 0 のプロセスはベクトルのメモリを確保 ...
    
    // 全プロセスにベクトルをブロードキャスト
    MPI_Bcast(vector, M, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    ```
  - **行列の行データをサイクリックに分配**
    ```c
    // rank 0: 各プロセスに行を送信
    if (world_rank == 0) {
        for (int i = 0; i < N; i++) {
            int dest_rank = i % world_size;
            if (dest_rank == 0) {
                // 自分自身へはメモリコピー
            } else {
                // 他のrankへノンブロッキング送信
                MPI_Isend(&matrix[i * M], M, MPI_DOUBLE, dest_rank, ...);
            }
        }
    } 
    // rank 0以外: 行を受信
    else {
        if (my_rows > 0) {
            for (int i = 0; i < N; i++) {
                if (i % world_size == world_rank) {
                    MPI_Irecv(&my_matrix_part[my_row_idx * M], M, MPI_DOUBLE, 0, ...);
                }
            }
            // 全ての受信完了を待つ
            MPI_Waitall(my_rows, recv_requests, MPI_STATUSES_IGNORE);
        }
    }
    ```

#### 4.  並列計算 (内積)
全プロセスが、それぞれに割り当てられたデータで内積計算を同時に実行します。

- **対応するコード (`1.c`)**
  ```c
  // --- 内積計算 ---
  double *my_results = (double *)malloc(my_rows * sizeof(double));
  for (int i = 0; i < my_rows; i++) {
      my_results[i] = 0.0;
      for (int j = 0; j < M; j++) {
          my_results[i] += my_matrix_part[i * M + j] * vector[j];
      }
  }
  ```

#### 5.  結果の集約
各プロセスが計算した結果を `rank 0` に集め、最終的な結果ベクトルを完成させます。

- **対応するコード (`1.c`)**
  - **rank 0以外: 計算結果を送信**
    ```c
    // ...
    } else { // world_rank != 0
        if(my_rows > 0) {
            // ...
            // 計算結果をノンブロッキング送信
            MPI_Isend(&my_results[send_req_count], 1, MPI_DOUBLE, 0, ...);
            // ...
        }
    }
    ```
  - **rank 0: 計算結果を受信**
    ```c
    if (world_rank == 0) {
        // ...
        for (int i = 0; i < N; i++) {
            int source_rank = i % world_size;
            if (source_rank == 0) {
                 // 自身の計算結果をコピー
            } else {
                // 他のrankからノンブロッキング受信
                MPI_Irecv(&result_vector[i], 1, MPI_DOUBLE, source_rank, ...);
            }
        }
        // ... 受信完了を待つループ ...
        print_vector("計算結果ベクトル:", N, result_vector);
    }
    ```

#### 6.  終了処理
全プロセスが確保したメモリを解放し、MPI環境を終了します。

- **対応するコード (`1.c`)**
  ```c
  // --- メモリ解放 ---
  if (world_rank == 0) {
      free(matrix);
      free(result_vector);
  }
  free(vector);
  free(my_matrix_part);
  free(my_results);
  free(recv_requests);

  MPI_Finalize();
  return 0;
  ```
---

## 主要なMPI関数の解説

このプログラムで使われている中心的なMPI関数について、引数を含めて解説します。

### `MPI_Bcast` (ブロードキャスト通信)
"One-to-All" (1対多) の通信で、1つのプロセスが持つデータを他の全プロセスにコピーします。

- **シグネチャ**:
  `int MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)`
- **引数の解説**:
    - `buffer`: 送受信するデータが格納されている（または受信後に格納される）変数のアドレス。
    - `count`: 送受信するデータ要素の数。
    - `datatype`: 送受信するデータの型 (`MPI_INT`, `MPI_DOUBLE` など)。
    - `root`: 送信元となるルートプロセスのランク番号。
    - `comm`: 通信が行われるコミュニケータ (`MPI_COMM_WORLD` など)。

### `MPI_Isend` (ノンブロッキング送信)
データ送信を開始し、完了を待たずに即座にリターンします。

- **シグネチャ**:
  `int MPI_Isend(const void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request)`
- **引数の解説**:
    - `buf`: 送信するデータが格納されているバッファのアドレス。
    - `count`: 送信するデータ要素の数。
    - `datatype`: 送信するデータの型。
    - `dest`: 送信先のプロセスのランク番号。
    - `tag`: メッセージを識別するための整数タグ。受信側と対にする必要があります。
    - `comm`: 通信が行われるコミュニケータ。
    - `request`: この通信操作を識別するためのリクエストオブジェクトへのポインタ。`MPI_Wait`系関数で完了を確認するために使います。

### `MPI_Irecv` (ノンブロッキング受信)
データ受信を開始し、完了を待たずに即座にリターンします。

- **シグネチャ**:
  `int MPI_Irecv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request)`
- **引数の解説**:
    - `buf`: 受信したデータを格納するバッファのアドレス。
    - `count`: 受信するデータ要素の**最大数**。
    - `datatype`: 受信するデータの型。
    - `source`: 受信元プロセスのランク番号。特定の相手から受信する場合はそのランクを、任意の相手でよければ `MPI_ANY_SOURCE` を指定します。
    - `tag`: メッセージ識別用のタグ。特定のタグを持つメッセージのみ受信します。`MPI_ANY_TAG` で任意のタグを受信できます。
    - `comm`: 通信が行われるコミュニケータ。
    - `request`: この通信操作を識別するためのリクエストオブジェクトへのポインタ。

### `MPI_Waitall` / `MPI_Wait` (完了待機)
ノンブロッキング通信 (`MPI_Isend`/`MPI_Irecv`) の完了を待機します。

- **`MPI_Waitall` のシグネチャ**:
  `int MPI_Waitall(int count, MPI_Request array_of_requests[], MPI_Status array_of_statuses[])`
- **引数の解説**:
    - `count`: `array_of_requests` に含まれるリクエストの数。
    - `array_of_requests`: 完了を待つ通信リクエストの配列。
    - `array_of_statuses`: 各通信の詳細情報（受信メッセージのサイズなど）を格納する `MPI_Status` 構造体の配列。不要な場合は `MPI_STATUSES_IGNORE` を指定できます。

- **`MPI_Wait` のシグネチャ**:
  `int MPI_Wait(MPI_Request *request, MPI_Status *status)`
- **引数の解説**:
    - `request`: 完了を待つ単一の通信リクエストへのポインタ。
    - `status`: 通信の詳細情報を格納する `MPI_Status` 構造体へのポインタ。不要な場合は `MPI_STATUS_IGNORE` を指定できます。

---

## MPIの実行モデルについて (SPMD)

MPIのプログラムは **SPMD (Single Program, Multiple Data)** というモデルで動作します。これは、`mpiexec -n 4` のように起動すると、**全く同じプログラムのコピーが4つ**（4つのプロセスとして）同時に実行されることを意味します。

各プロセスは、コードの大部分を共通して実行しますが、自身の `world_rank`（`0` から始まるユニークな識別番号）を使って、自分が実行すべき処理を判断します。

- **`if (world_rank == ...)` の外側**:
  ここに記述されたコードは、**全プロセスが実行**します。例えば、内積計算のロジック自体は全プロセスで共通ですが、計算に使うデータ（担当する行列の行）はプロセスごとに異なります。

- **`if (world_rank == 0)` の内側**:
  このブロックは、**`rank 0` のプロセスだけが実行**します。ファイル読み込み、全体の処理のとりまとめ、最終結果の出力といった、1つのプロセスに集約させたい処理を記述するのが一般的です。

- **`else` や `if (world_rank != 0)` の内側**:
  このブロックは、**`rank 0` 以外の全プロセス**（`rank 1`, `rank 2`, ...）が実行します。`rank 0` からデータを受信する処理などが該当します。

このように、全プロセスが同じプログラムを実行しつつ、`world_rank` による条件分岐を用いて各プロセスに異なる役割を与えることで、協調的な並列処理を実現しています。
