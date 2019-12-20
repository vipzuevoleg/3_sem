#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handler_recipient(int sig) {
    static int bitcount = 0;
    static char data[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    switch (sig)
    {
    case SIGUSR1:
        bitcount++;
        break;
    
    case SIGUSR2:
        data[bitcount / 8] |= (1 << (bitcount % 8));
        bitcount++;
        break;
    
    case SIGINT:
        printf("recieved message: %s\n", data);
        exit(0);
    }
    kill(getppid(), SIGUSR1);
}

int pid;
void handler_sender(int sig) {
    static int bitcount = -1;
    static char src[13] = "test message";
    bitcount++;
    if(bitcount / 8 == 13) {
        kill(pid, SIGINT);
        exit(0);
    }
    if(src[bitcount / 8] & (1 << (bitcount % 8)))
        kill(pid, SIGUSR2);
    else
        kill(pid, SIGUSR1);
}

int main() {
    pid = fork();
    if(pid) {
        signal(SIGUSR1, handler_sender);
    }
    else {
        signal(SIGUSR1, handler_recipient);
        signal(SIGUSR2, handler_recipient);
        signal(SIGINT, handler_recipient);
        sleep(1);
        kill(getppid(), SIGUSR1);
    }
    while(1) sleep(1);
}