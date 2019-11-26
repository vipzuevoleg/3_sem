#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    char* command = malloc(sysconf(_SC_ARG_MAX));
    
    while(1) {
        
        printf("enter your command: ");
        fgets(command, sysconf(_SC_ARG_MAX), stdin);
        command[strlen(command) - 1] = 0;

        if(!strcmp(command, "exit"))
            return 0;

        int cpid = fork();
        if(cpid) {
            int exitstatus;
            waitpid(cpid, &exitstatus, 0);
            printf("WEXITSTATUS: %d\n", WEXITSTATUS(exitstatus));
        }
        else {
            char* arguments[99999];
            
            int i = 0;
            arguments[0] = strtok(command, " ");
            while(arguments[++i] = strtok(NULL, " "));
                
            execvp(arguments[0], arguments);
            printf("execvp didn't work\n");
            return 1;
        }
    }
}

