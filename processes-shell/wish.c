#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>


#define MAXCMD 512

int main(int argc, char *argv[]) {

    printf("wish> ");

    char *line = NULL;
    size_t len = 0;
    
    // ssize_t nread;
    // while ((nread = getline(&line, &len, stdin)) != -1) {}
    getline(&line, &len, stdin);

    char* cmd_argv[len];
    const char *delim = ",;/ ";
    //char *strtok(char *str, const char *delim);
    char* token = strtok(line, delim);
    size_t index = 0;
    while(token != NULL) {
        cmd_argv[index] = token;
        // printf("cmd_argv[%ld]: %s\n", index, cmd_argv[index]);
        index++;
        // Subsequent calls: pass NULL to keep parsing the same string
        token = strtok(NULL, delim); 
    }
    cmd_argv[index] = NULL;
    free(line);


    // execv(cmd_argv[0], cmd_argv);
    // // int execv(const char *pathname, char *const argv[]);
    // // if successful, doesn't return
    // printf("failed\n");


    

















    // int rc = fork();
    // int x = 10;


    // // creates new process
    // if(rc == 0) {
    //     // child
    //     printf("child pid: %d %d\n", (int) getpid(), x);

    //     char* cmd_argv[argc];
    //     cmd_argv[0] = strdup("/bin/ls");
    //     cmd_argv[1] = strdup("-l");
    //     cmd_argv[2] = NULL;
    
    //     execv(cmd_argv[0], cmd_argv);
    //     // int execv(const char *pathname, char *const argv[]);
    //     // if successful, doesn't return
    //     printf("failed\n");



    // } else if (rc > 0) {
    //     // parent
    //     (void) wait(NULL);
    //     printf("parent pid: %d, child pid: %d x: %d\n", (int)getpid(), rc, x);
    // } else {
    //     // failure
    // }

    return 0;
}
