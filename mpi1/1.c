#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Function prototypes
int Size_count(char* vector_file);
void Memory_allocate(double** d1, int N);
void File_read(double* d1, int N, char* vec_file);
void bai(double* a, int N);
void printa(double* a, int N);
void printb(double* a, int N);

int main(int argc, char* argv[]) {
    int N, n, p;
    double* a;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &p);

    N = Size_count(argv[1]);
    Memory_allocate(&a, N);
    File_read(a, N, argv[1]);

    if (p == 0) {
        MPI_Send(a, N, MPI_DOUBLE, 1, 9, MPI_COMM_WORLD);
        MPI_Recv(a, N, MPI_DOUBLE, 1, 10, MPI_COMM_WORLD, &status);
        printb(a, N);
    } else if (p == 1) {
        MPI_Recv(a, N, MPI_DOUBLE, 0, 9, MPI_COMM_WORLD, &status);
        printa(a, N);
        bai(a, N);
        MPI_Send(a, N, MPI_DOUBLE, 0, 10, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    free(a);
    return 0;
}

int Size_count(char* vector_file) {
    FILE* fp;
    char c;
    int N = 0;

    printf("Vector file: %s\n", vector_file);
    if ((fp = fopen(vector_file, "r")) == NULL) {
        printf("Vector file not found!!\n");
        exit(1);
    }

    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            N++;
        }
    }

    fclose(fp);
    return N;
}

void Memory_allocate(double** d1, int N) {
    if ((*d1 = (double*)malloc(N * sizeof(double))) == NULL) {
        printf("No memories are available (b)");
        exit(1);
    }
}

void File_read(double* d1, int N, char* vec_file) {
    FILE* fp;
    int i;

    fp = fopen(vec_file, "r");
    for (i = 0; i < N; i++) {
        if (fscanf(fp, "%lf", &d1[i]) == EOF) {
            printf("vector file read error b[%d]\n", i);
            exit(1);
        }
    }
    fclose(fp);
}

void bai(double* a, int N) {
    int i;
    for (i = 0; i < N; i++) {
        a[i] *= 2;
    }
}

void printa(double* a, int N) {
    int i;
    for (i = 0; i < N; i++) {
        printf("a[%d]=%lf\n", i, a[i]);
    }
}

void printb(double* a, int N) {
    int i;
    for (i = 0; i < N; i++) {
        printf("b[%d]=%lf\n", i, a[i]);
    }
}
