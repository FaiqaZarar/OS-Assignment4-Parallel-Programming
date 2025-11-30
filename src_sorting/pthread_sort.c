#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int *arr;
    int left;
    int right;
} thread_args_t;

int active_threads = 0;
int max_threads = 4;
pthread_mutex_t thread_count_mutex = PTHREAD_MUTEX_INITIALIZER;

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

void* parallel_merge_sort(void* args) {
    thread_args_t *targs = (thread_args_t*)args;
    int left = targs->left;
    int right = targs->right;
    int *arr = targs->arr;
    
    if (left < right) {
        int mid = left + (right - left) / 2;
        
        pthread_mutex_lock(&thread_count_mutex);
        int can_spawn = (active_threads < max_threads);
        if (can_spawn) active_threads += 2;
        pthread_mutex_unlock(&thread_count_mutex);
        
        if (can_spawn) {
            pthread_t left_thread, right_thread;
            thread_args_t left_args = {arr, left, mid};
            thread_args_t right_args = {arr, mid + 1, right};
            
            pthread_create(&left_thread, NULL, parallel_merge_sort, &left_args);
            pthread_create(&right_thread, NULL, parallel_merge_sort, &right_args);
            
            pthread_join(left_thread, NULL);
            pthread_join(right_thread, NULL);
            
            pthread_mutex_lock(&thread_count_mutex);
            active_threads -= 2;
            pthread_mutex_unlock(&thread_count_mutex);
        } else {
            sequential_merge_sort(arr, left, mid);
            sequential_merge_sort(arr, mid + 1, right);
        }
        
        merge(arr, left, mid, right);
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <array_size> <num_threads>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    max_threads = atoi(argv[2]);
    
    printf("Pthread Merge Sort: Array size = %d, Threads = %d\n", n, max_threads);
    
    int *arr = (int*)malloc(n * sizeof(int));
    srand(time(NULL));
    
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 100000;
    }
    
    clock_t start = clock();
    thread_args_t args = {arr, 0, n - 1};
    parallel_merge_sort(&args);
    clock_t end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Execution Time: %.6f seconds\n", time_taken);
    
    free(arr);
    pthread_mutex_destroy(&thread_count_mutex);
    return 0;
}
