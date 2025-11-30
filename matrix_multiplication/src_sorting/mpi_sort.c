#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

void merge(int arr[], int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    int *L = (int*)malloc(n1 * sizeof(int));
    int *R = (int*)malloc(n2 * sizeof(int));
    
    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];
    
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }
    
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    
    free(L);
    free(R);
}

void merge_sort(int arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort(arr, left, mid);
        merge_sort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void merge_arrays(int *arr1, int n1, int *arr2, int n2, int *result) {
    int i = 0, j = 0, k = 0;
    
    while (i < n1 && j < n2) {
        if (arr1[i] <= arr2[j]) {
            result[k++] = arr1[i++];
        } else {
            result[k++] = arr2[j++];
        }
    }
    
    while (i < n1) result[k++] = arr1[i++];
    while (j < n2) result[k++] = arr2[j++];
}

int main(int argc, char *argv[]) {
    int rank, size, n;
    double start_time, end_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc != 2) {
        if (rank == 0) {
            printf("Usage: mpirun -np <num_processes> %s <array_size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    n = atoi(argv[1]);
    int *arr = NULL;
    
    if (rank == 0) {
        printf("MPI Merge Sort: Array size = %d, Processes = %d\n", n, size);
        arr = (int*)malloc(n * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < n; i++) {
            arr[i] = rand() % 100000;
        }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    
    int local_n = n / size;
    int *local_arr = (int*)malloc(local_n * sizeof(int));
    
    MPI_Scatter(arr, local_n, MPI_INT, local_arr, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    
    merge_sort(local_arr, 0, local_n - 1);
    
    int step = 1;
    while (step < size) {
        if (rank % (2 * step) == 0) {
            if (rank + step < size) {
                int recv_size = local_n * step;
                int *recv_arr = (int*)malloc(recv_size * sizeof(int));
                
                MPI_Recv(recv_arr, recv_size, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                int *merged = (int*)malloc((local_n + recv_size) * sizeof(int));
                merge_arrays(local_arr, local_n, recv_arr, recv_size, merged);
                
                free(local_arr);
                free(recv_arr);
                local_arr = merged;
                local_n += recv_size;
            }
        } else {
            int dest = rank - step;
            MPI_Send(local_arr, local_n, MPI_INT, dest, 0, MPI_COMM_WORLD);
            break;
        }
        step *= 2;
    }
    
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("Execution Time: %.6f seconds\n", end_time - start_time);
        free(arr);
    }
    
    free(local_arr);
    MPI_Finalize();
    return 0;
}
