#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int sum = 0;
pthread_mutex_t _mutex;

struct Data {
    int batchsize;
    int* v;
};

void* routine(void *param) {
    struct Data *input = (struct Data*)param;
    int sum_loc = 0;

    for(int i = 0; i < input->batchsize; i++) {
        sum_loc += input->v[i];
    }

    pthread_mutex_lock(&_mutex);
    sum += sum_loc;
    pthread_mutex_unlock(&_mutex);
    
    return 0;
}

int vector_sum(int* v, int size, int nthreads) {
    pthread_t *threads = (pthread_t*) malloc(nthreads*sizeof(pthread_t));
    struct Data *inputs = (struct Data*) malloc(nthreads*sizeof(struct Data));

    int batchsize = size / nthreads + 1*(!!(size % nthreads));

    for(int i = 0; i < nthreads; i++) {
        int batchsize_loc = batchsize;
        if (i == nthreads - 1) {
            batchsize_loc = size - batchsize*(nthreads - 1);
        }
        inputs[i].batchsize = batchsize_loc;
        inputs[i].v = v + i * batchsize_loc;

        pthread_create(threads + i, NULL, &routine, (void *)&inputs[i]);
    }

    for(int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(inputs);

    return sum;
}

int main() {
    int nthreads = 3;
    int size = 100000;

    int* arr = malloc(size*sizeof(int));
    for (int i = 0; i < size; i++) {
        arr[i] = 1;
    }

    int total = vector_sum(arr, size, nthreads);

    printf("%d\n", total);

    free(arr);
}