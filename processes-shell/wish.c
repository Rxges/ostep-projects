#include <stdio.h>
#include <unistd.h>


#define MAXCMD 512

int main(int argc, char *argv[]) {
    int rc = fork();
    int x = 10;


    // creates new process
    if(rc == 0) {
        // child
        printf("child pid: %d %d\n", (int) getpid(), x);
        

        char* cmd_argv[10];
        cmd_argv[0] = strdup("/bin/ls");
        cmd_argv[1] = strdup("-l");
        cmd_argv[2] = NULL;

        execv(cmd_argv[0], cmd_argv);
        // int execv(const char *pathname, char *const argv[]);
        // if successful, doesn't return
        printf("failed\n");



    } else if (rc > 0) {
        // parent
        (void) wait(NULL);
        printf("parent pid: %d, child pid: %d x: %d\n", (int)getpid(), rc, x);
    } else {
        // failure
    }

    return 0;
}
