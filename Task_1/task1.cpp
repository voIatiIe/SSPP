#include <iostream>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#define PI 3.14159265358979323846

using namespace std;


struct Data{
    double left;
    long int nSteps;
};

const double LEFT = 0.0;
const double RIGHT = 1.0;
double step, pi = 0;
pthread_mutex_t _mutex;

double itg_func(double x) {
    return 4.0/(1 + x*x);
}

void* routine(void *param) {
    Data *input = (Data*)param;
    double left = input->left, sum = 0;

    for(long int i = 0; i < input->nSteps; i++) {
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
        cout << "Not enough arguments" << endl;
        return 1;
    }
    long int nIntervals = stol(argv[1]);
    int nThreads = stoi(argv[2]);

    cout << "nThreads: " << nThreads << endl;
    cout << "nIntervals: " << nIntervals << endl;

    step = (RIGHT - LEFT)/nIntervals;
    long int optSteps = nIntervals / nThreads + 1*(!!(nIntervals % nThreads));
    cout << "Intervals per thread: " << optSteps << endl;

    pthread_t *threads = new pthread_t[nThreads];
    Data *inputs = new Data[nThreads];

    pthread_mutex_init(&_mutex, NULL);

    double left = LEFT;
    timeval tval;
    gettimeofday(&tval, NULL);
    long int t0 = 1.0e6 * tval.tv_sec + tval.tv_usec;

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

    gettimeofday(&tval, NULL);
    long int t1 = 1.0e6 * tval.tv_sec + tval.tv_usec;

    pthread_mutex_destroy(&_mutex);
    delete [] threads;
    delete [] inputs;

    printf("PI: %.20f\n", pi);
    printf("dPI: %.20f\n", pi-PI);
    printf("Elapsed time(us): %ld\n", t1 - t0);
}