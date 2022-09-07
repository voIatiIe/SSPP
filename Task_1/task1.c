#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

struct Data {
    double left;
    long nSteps;
};
const double LEFT = 0.0;
const double RIGHT = 1.0;
double step, pi = 0;
pthread_mutex_t _mutex;


double itg_func(double x) { return 4.0/(1 + x*x); }

long get_time_us() {
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return (1.0e6 * tval.tv_sec + tval.tv_usec);
}

void* routine(void *param) {
    struct Data *input = (struct Data*)param;
    double left = input->left, sum = 0;

    for(long i = 0; i < input->nSteps; i++) {
        sum += step*itg_func(left + step/2);
        left += step;
    }
    
    pthread_mutex_lock(&_mutex);
    pi += sum;
    pthread_mutex_unlock(&_mutex);

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        return 1;
    }

    long nIntervals = strtol(argv[1], NULL, 10);
    long nThreads = strtol(argv[2], NULL, 10);
    double left = LEFT;
    long optSteps = nIntervals / nThreads + 1*(!!(nIntervals % nThreads));    

    pthread_t *threads = (pthread_t*) malloc(nThreads*sizeof(pthread_t));
    struct Data *inputs = (struct Data*) malloc(nThreads*sizeof(struct Data));

    pthread_mutex_init(&_mutex, NULL);
    step = (RIGHT - LEFT)/nIntervals;

    long t0 = get_time_us();
    for(int i = 0; i < nThreads; i++) {
        inputs[i].left = left;
        inputs[i].nSteps = optSteps;
        if (i == nThreads - 1) inputs[i].nSteps = nIntervals - optSteps*(nThreads - 1);
        left += optSteps*step;

        pthread_create(threads + i, NULL, &routine, (void *)&inputs[i]);
    }

    for(int i = 0; i < nThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("PI: %.20f\n", pi);
    printf("Elapsed time(us): %ld\n", get_time_us() - t0);

    pthread_mutex_destroy(&_mutex);
    free(threads);
    free(inputs);
}