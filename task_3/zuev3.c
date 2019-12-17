#define _GNU_SOURCE
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>

long long min(long long a, long long b) {
    return (a < b) ? a : b;
}

double get_time() {
    struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec + currentTime.tv_usec * 1e-6;
}

char* data;
long long size = 0;
int buffer_size = 4096;
char* buffer;

void fifo_send(char* output_filename) {
    double t0 = get_time();
    int fifo[2];
    pipe(fifo);
    int pid = fork();
    if(pid) {
        for(long long i = 0; i < size; i += buffer_size)
            write(fifo[1], &data[i], min(buffer_size, size - i));
        waitpid(pid, NULL, 0);
    }
    else {
        int output = open(output_filename, O_WRONLY | O_CREAT, 0666);
        for(long long i = 0; i < size; ) {
            int delta = read(fifo[0], buffer, min(buffer_size, size - i));
            write(output, buffer, delta);
            i += delta;
        }
        exit(0);
    }
    printf("fifo transfer: %lfs\n", get_time() - t0);
}
typedef struct {
    long mtype;
    char mtext[8192];
} msgbuf;
void queue_send(char* output_filename) {
    double t0 = get_time();
    int qid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    msgbuf buf;
    buf.mtype = 5;
    int pid = fork();
    if(pid) {
        for(long long i = 0; i < size; i += buffer_size) {
            memcpy(buf.mtext, &data[i], min(buffer_size, size - i));
            msgsnd(qid, &buf, buffer_size, 0);
        }
        waitpid(pid, NULL, 0);
    }
    else {
        int output = open(output_filename, O_WRONLY | O_CREAT, 0666);
        for(long long i = 0; i < size; i += buffer_size) {
            msgrcv(qid, &buf, buffer_size, 5, 0);
            write(output, buf.mtext, min(buffer_size, size - i));
        }
        exit(0);
    }
    printf("queue transfer: %lfs\n", get_time() - t0);
}

void shm_send(char* output_filename) {
    double t0 = get_time();
    char* shm = shmat(shmget(IPC_PRIVATE, buffer_size, IPC_CREAT | 0666), NULL, 0);
    sem_t* sems[2] = {sem_open("/sem0", O_CREAT, 0666, 0), sem_open("/sem1", O_CREAT, 0666, 0)};
    int pid = fork();
    if(pid) {
        for(long long i = 0; i < size; i+=buffer_size) {
            memcpy(shm, &data[i], min(buffer_size, size - i));
            sem_post(sems[0]);
            sem_wait(sems[1]);
        }
        waitpid(pid, NULL, 0);
    }
    else {
        int output = open(output_filename, O_WRONLY | O_CREAT, 0666);
        for(long long i = 0; i < size; i+=buffer_size) {
            sem_wait(sems[0]);
            write(output, shm, min(buffer_size, size - i));
            sem_post(sems[1]);
        }
        exit(0);
    }
    sem_destroy(sems[0]);
    sem_destroy(sems[1]);
    printf("shm transfer: %lfs\n", get_time() - t0);
} 

int main() {
    srand(time(NULL));
    int input = open("data_2700mb", O_RDONLY);
    size = 0;
    long long data_size = buffer_size;
    data = malloc(data_size);
    buffer = malloc(buffer_size);

    int buff_size;
    while((buff_size = read(input, buffer, buffer_size)) > 0) {
        if(size + buff_size > data_size)
            data = realloc(data, data_size *= 2);
        memcpy(&data[size], buffer, buff_size);
        size += buff_size;
    }
    int buffer_sizes[6] = {32,128,1024,4096,8192,65536};
    for(int i = 0; i < 6; i++) {
        buffer_size = buffer_sizes[i];
        printf("buffer size: %d\n", buffer_size);
        fifo_send("data_2700mb_fifo");    
        shm_send("data_2700mb_shm");  
        if(buffer_size <= 8192)
            queue_send("data_2700mb_queue");    
    }  
}