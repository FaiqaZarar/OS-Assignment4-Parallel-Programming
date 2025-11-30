#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define THRESHOLD 10000

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

void sequential_merge_sort(int arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        sequential_merge_sort(arr, left, mid);
        sequential_merge_sort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void parallel_merge_sort(int arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        int size = right - left + 1;
        
        if (size > THRESHOLD) {
            #pragma omp task shared(arr)
            parallel_merge_sort(arr, left, mid);
            
            #pragma omp task shared(arr)
            parallel_merge_sort(arr, mid + 1, right);
            
            #pragma omp taskwait
        } else {
            sequential_merge_sort(arr, left, mid);
            sequential_merge_sort(arr, mid + 1, right);
        }
        
        merge(arr, left, mid, right);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <array_size> <num_threads>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    
    printf("OpenMP Merge Sort: Array size = %d, Threads = %d\n", n, num_threads);
    
    int *arr = (int*)malloc(n * sizeof(int));
    srand(time(NULL));
    
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 100000;
    }
    
    omp_set_num_threads(num_threads);
    
    double start = omp_get_wtime();
    
    #pragma omp parallel
    {
        #pragma omp single
        parallel_merge_sort(arr, 0, n - 1);
    }
    
    double end = omp_get_wtime();
    double time_taken = end - start;
    
    printf("Execution Time: %.6f seconds\n", time_taken);
    
    free(arr);
    return 0;
}
