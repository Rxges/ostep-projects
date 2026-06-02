#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>


#define MAXCMD 512

int main(int argc, char *argv[]) {

    while(1) {
        printf("wish> ");
        
        char *line = NULL;
        size_t len = 0;
        
        // ssize_t nread;
        // while ((nread = getline(&line, &len, stdin)) != -1) {
        //     // printf("wish> ");
        //     printf("Retrieved line of length %zd:\n", nread);
        // }
        getline(&line, &len, stdin);

        char* cmd_argv[len];
        const char *delim = ",; \n";
        //char *strtok(char *str, const char *delim);
        char* token = strtok(line, delim);
        size_t index = 0;
        while(token != NULL) {
            // printf("token: %s\n", token);
            
            cmd_argv[index] = strdup(token);
            // printf("cmd_argv[%ld]: %s\n", index, cmd_argv[index]);
            index++;
            // Subsequent calls: pass NULL to keep parsing the same string
            token = strtok(NULL, delim); 
            
        }
        cmd_argv[index] = NULL;
        
        free(line);

        if(strcmp(cmd_argv[0], "exit") == 0) {
            break;
        } else {
            int rc = fork();
            
            // creates new process
            if(rc == 0) {
                // child
                execvp(cmd_argv[0], cmd_argv);
                printf("failed\n"); // if successful, doesn't return

            } else if (rc > 0) {
                // parent
                (void) wait(NULL);
                // printf("parent pid: %d, child pid: %d\n", (int)getpid(), rc);
            } else {
                // failure
            }
        }
    }

    return 0;
}
