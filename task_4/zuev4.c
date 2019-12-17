#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

double get_time() {
    struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec + currentTime.tv_usec * 1e-6;
}
typedef struct {
    double x_start, x_end;
    struct drand48_data data;
    int n;
    double res;
} args_t;
double get_rand(struct drand48_data* data) {
    double d;
    drand48_r(data, &d);
    return d;
}
double f(double x) {
    return 1 - (x - 1) * (x - 1);
}
void* routine(void* _args) {
    args_t* args = _args;
    for(int i = 0; i < args->n; i++)
        args->res += f(args->x_start + (args->x_end - args->x_start) * get_rand(&(args->data))) * (args->x_end - args->x_start);
}
int main() {
    int npoints = 3*5*7*0x100000;
    printf("threads\ttime\tresult\n");
    for(int i = 1; i < 0x8000; i++) {
        double t0 = get_time();
        if(npoints % i != 0)
            continue;
        args_t data[i];
        for(int j = 0; j < i; j++) {
            data[j].x_start = 0;
            data[j].x_end = 2;
            data[j].n = npoints / i;
            data[j].res = 0;
            srand48_r(rand(), &(data[j].data));
        }
        pthread_t threads[i];
        for(int j = 0; j < i; j++)
            pthread_create(&threads[j], NULL, routine, &data[j]);
        double res = 0;
        for(int j = 0; j < i; j++) {
            pthread_join(threads[j], NULL);
            res += data[j].res / npoints;
        }
        printf("%d\t%.05lf\t%lf\n", i, get_time() - t0, res);
    }
}