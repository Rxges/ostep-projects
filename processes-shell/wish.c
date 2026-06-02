#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {

    FILE *stream = NULL;
    if(argc == 2) {
        stream = fopen(argv[1], "r");
        if (stream == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    }

    while(1) {
        char *line = NULL;
        size_t len = 0;
        ssize_t nread = 0;
        
        if(argc != 2) { // interactive mode
            // read from standard input
            printf("wish> ");
            nread = getline(&line, &len, stdin); 
        } else { // batch mode
            // read from file
            nread = getline(&line, &len, stream); 
        }

        char* cmd_argv[len];
        const char *delim = ",; \n";
        char* token = strtok(line, delim); //char *strtok(char *str, const char *delim);
        // char* token = strsep(&line, delim); //char *strsep(char **stringp, const char *delim);
        size_t index = 0;
        while(token != NULL) {            
            cmd_argv[index] = strdup(token);
            // printf("cmd_argv[%ld]: %s\n", index, cmd_argv[index]);
            index++;
            // Subsequent calls: pass NULL to keep parsing the same string
            token = strtok(NULL, delim); 
            // token = strsep(&line, delim); 
        }
        cmd_argv[index] = NULL;
        
        free(line);

        if(nread == -1) { // hit end-of-file marker (EOF)
            if(stream) {
                // if read from file
                fclose(stream);
            }
            exit(0);
        }

        if(cmd_argv[0] != NULL) { // ensures there was an input (avoids seg fault if you just press enter)
            if(strcmp(cmd_argv[0], "exit") == 0) {
                if(stream) {
                    // if read from file
                    fclose(stream);
                }
                exit(0);
            } else {
                int rc = fork();
                
                // creates new process
                if(rc == 0) {
                    // child
                    execvp(cmd_argv[0], cmd_argv);
                    // execv(cmd_argv[0], cmd_argv);
                    printf("An error has occurred\n"); // if successful, doesn't return (aka doesn't print)

                } else if (rc > 0) {
                    // parent
                    (void) wait(NULL);
                    // printf("parent pid: %d, child pid: %d\n", (int)getpid(), rc);
                } else {
                    // failure
                }
            }
        }
    }

    return 0;
}
