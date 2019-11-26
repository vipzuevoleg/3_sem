#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

typedef struct {
  int p[2][2];
} dpipe_t;

int main() {
    dpipe_t p;
    pipe(p.p[0]);
    pipe(p.p[1]);
    int cpid = fork();

    while(1) {
        if(cpid) {
            close(p.p[0][0]);
            close(p.p[1][1]);
            
            char message[99999];
            printf("what to send: ");
            scanf("%s", message);

            if(!strcmp(message, "exit"))
                exit(0);

            int response;
            dprintf(p.p[0][1], "%s\n", message);
            fscanf(fdopen(p.p[1][0], "r"), "%d", &response);
            printf("child responded: %d\n", response);
        }
        else {
            close(p.p[1][0]);
            close(p.p[0][1]);

            char message[99999];
            fscanf(fdopen(p.p[0][0], "r"), "%s", message);
            dprintf(p.p[1][1], "%d\n", strlen(message));
        }
    }
}