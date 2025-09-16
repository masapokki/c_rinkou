#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ファイルから行列の次元（行数と列数）を取得する関数
void get_matrix_dimensions(const char* filename, int* rows, int* cols) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "エラー: ファイルを開けません %s\n", filename);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    *rows = 0;
    *cols = 0;
    char line[1024]; 
    int cols_in_row = 0;
    char* token;

    if (fgets(line, sizeof(line), file) != NULL) {
        (*rows)++;
        token = strtok(line, " \t\n");
        while(token != NULL) {
            if(strlen(token) > 0) (*cols)++;
            token = strtok(NULL, " \t\n");
        }
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if(strlen(line) > 1) (*rows)++;
    }

    fclose(file);
}

// ファイルからベクトルのサイズ（要素数）を取得する関数
void get_vector_size(const char* filename, int* size) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "エラー: ファイルを開けません %s\n", filename);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    *size = 0;
    double temp;
    while (fscanf(file, "%lf", &temp) == 1) {
        (*size)++;
    }
    fclose(file);
}


// 行列を整形して出力する関数
void print_matrix(const char* title, int rows, int cols, double *matrix) {
    printf("%s\n", title);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%8.2f", matrix[i * cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}

// ベクトルを整形して出力する関数
void print_vector(const char* title, int size, double *vector) {
    printf("%s\n", title);
    for (int i = 0; i < size; i++) {
        printf("%8.2f\n", vector[i]);
    }
    printf("\n");
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int N = 0, M = 0; // 行列の次元 (N行, M列)
    double *matrix = NULL;
    double *vector = NULL;
    double *result_vector = NULL;
    MPI_Request *send_requests = NULL;

    if (world_rank == 0) {
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

        if (M != vec_dim) {
             fprintf(stderr, "エラー: 行列とベクトルの次元が非互換です。 (M=%d, vec_dim=%d)\n", M, vec_dim);
             MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // 行列のメモリ確保と読み込み
        matrix = (double *)malloc(N * M * sizeof(double));
        FILE *matrix_file = fopen(matrix_filename, "r");
        for (int i = 0; i < N * M; i++) {
            if (fscanf(matrix_file, "%lf", &matrix[i]) != 1) {
                fprintf(stderr, "エラー: 行列のデータを読み込めません。\n");
                fclose(matrix_file);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
        fclose(matrix_file);

        // ベクトルのメモリ確保と読み込み
        vector = (double *)malloc(M * sizeof(double));
        FILE *vector_file = fopen(vector_filename, "r");
        for (int i = 0; i < M; i++) {
            if (fscanf(vector_file, "%lf", &vector[i]) != 1) {
                fprintf(stderr, "エラー: ベクトルのデータを読み込めません。\n");
                fclose(vector_file);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
        fclose(vector_file);
        
        print_matrix("読み込み行列:", N, M, matrix);
        print_vector("読み込みベクトル:", M, vector);

        result_vector = (double *)malloc(N * sizeof(double));
    }

    // 全プロセスに次元をブロードキャスト
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // rank 0以外のプロセスでベクトルのメモリを確保
    if (world_rank != 0) {
        vector = (double *)malloc(M * sizeof(double));
    }
    // 全プロセスにベクトルをブロードキャスト
    MPI_Bcast(vector, M, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // --- 行列の各行をサイクリックに分配 ---
    
    // このプロセスが担当する行数を計算
    int my_rows = 0;
    for (int i = 0; i < N; i++) {
        if (i % world_size == world_rank) {
            my_rows++;
        }
    }
    
    if (my_rows == 0 && N > 0) {
        // 仕事がないプロセス。何もしないが、後続の処理で問題が起きないようにする。
    }


    double *my_matrix_part = (double *)malloc(my_rows * M * sizeof(double));
    MPI_Request *recv_requests = (MPI_Request *)malloc(my_rows * sizeof(MPI_Request));
    int my_row_idx = 0;

    if (world_rank == 0) {
        int send_req_count = 0;
        if (N > 0) { // N=0 のケースを考慮
             send_requests = (MPI_Request *)malloc((N - my_rows) * sizeof(MPI_Request));
        }
        for (int i = 0; i < N; i++) {
            int dest_rank = i % world_size;
            if (dest_rank == 0) {
                // 自身の担当分は直接コピー
                if (my_rows > 0) {
                    memcpy(&my_matrix_part[my_row_idx * M], &matrix[i * M], M * sizeof(double));
                    my_row_idx++;
                }
            } else {
                // 他のrankへノンブロッキング送信
                MPI_Isend(&matrix[i * M], M, MPI_DOUBLE, dest_rank, i, MPI_COMM_WORLD, &send_requests[send_req_count++]);
            }
        }
    } else {
        if (my_rows > 0) {
            for (int i = 0; i < N; i++) {
                if (i % world_size == world_rank) {
                    // 自身の担当分をノンブロッキング受信
                    MPI_Irecv(&my_matrix_part[my_row_idx * M], M, MPI_DOUBLE, 0, i, MPI_COMM_WORLD, &recv_requests[my_row_idx]);
                    my_row_idx++;
                }
            }
            // 受信完了を待つ
            MPI_Waitall(my_rows, recv_requests, MPI_STATUSES_IGNORE);
        }
    }
    
    // --- 内積計算 ---
    double *my_results = (double *)malloc(my_rows * sizeof(double));
    for (int i = 0; i < my_rows; i++) {
        my_results[i] = 0.0;
        for (int j = 0; j < M; j++) {
            my_results[i] += my_matrix_part[i * M + j] * vector[j];
        }
    }
    
    // --- 計算結果をrank 0に集約 ---
    if (world_rank == 0) {
        MPI_Request *result_recv_reqs = (MPI_Request*) malloc(N * sizeof(MPI_Request));
        int own_res_idx = 0;
        for (int i = 0; i < N; i++) {
            int source_rank = i % world_size;
            if (source_rank == 0) {
                 if(my_rows > 0) result_vector[i] = my_results[own_res_idx++];
            } else {
                // 他のrankからノンブロッキング受信
                MPI_Irecv(&result_vector[i], 1, MPI_DOUBLE, source_rank, i, MPI_COMM_WORLD, &result_recv_reqs[i]);
            }
        }
        // 受信完了を待つ (自分以外のrankからのものだけ)
        for (int i = 0; i < N; i++) {
            if (i % world_size != 0) {
                MPI_Wait(&result_recv_reqs[i], MPI_STATUS_IGNORE);
            }
        }
        free(result_recv_reqs);
        
        print_vector("計算結果ベクトル:", N, result_vector);

    } else {
        if(my_rows > 0) {
            MPI_Request *result_send_reqs = (MPI_Request*) malloc(my_rows * sizeof(MPI_Request));
            int send_req_count = 0;
            for (int i = 0; i < N; i++) {
                if (i % world_size == world_rank) {
                    // 計算結果をノンブロッキング送信
                    MPI_Isend(&my_results[send_req_count], 1, MPI_DOUBLE, 0, i, MPI_COMM_WORLD, &result_send_reqs[send_req_count]);
                    send_req_count++;
                }
            }
            // 送信完了を待つ
            MPI_Waitall(my_rows, result_send_reqs, MPI_STATUSES_IGNORE);
            free(result_send_reqs);
        }
    }

    // rank 0 で行列送信の完了を待つ
    if (world_rank == 0 && N > my_rows) {
        MPI_Waitall(N - my_rows, send_requests, MPI_STATUSES_IGNORE);
        free(send_requests);
    }

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
}
