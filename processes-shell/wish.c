#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>

#define MAXCMD 32
char* pathDirs[MAXCMD];
int pathCount = 0;

void error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
}

void freeStrdup(char** arr) {
    size_t i = 0;
    while(arr[i] != NULL) {
        free(arr[i]);  // strdup uses malloc to allocate string onto the heap
        arr[i] = NULL;
        i++;
    }
}

int main(int argc, char *argv[]) {

    FILE *stream = NULL;
    if(argc == 2) {
        stream = fopen(argv[1], "r");
        if (stream == NULL) {
            // file or directory in argv[1] is nonexistent
            error();
            exit(1);
        }
    } else if(argc > 2) {
        // if the shell is invoked with more than one file, or if the shell is 
        // passed a bad batch file, it should exit by calling exit(1)
        error();
        exit(1);
    }

    // typedef struct List {
    //     char* str;
    //     struct List* next;
    // } List;
    // List* pathList = malloc(sizeof(List)); // make sure to free pathList later
    pathDirs[0] = strdup("/bin");
    pathCount = 1;
    pathDirs[1] = NULL;

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
    
        if(nread == -1) { // hit end-of-file marker (EOF)
            if(stream) {
                // if read from file
                fclose(stream);
            }
            free(line);
            freeStrdup(pathDirs);
            exit(0);
        }

        char* cmd_argv[len];
        const char *delim = ",; \n";
        char* token = strtok(line, delim); //char *strtok(char *str, const char *delim);
        // char* token = strsep(&line, delim); //char *strsep(char **stringp, const char *delim);
        size_t index = 0;
        char* redirPtr = NULL;
        // char* outputFile = NULL;
        bool err = false;
        bool redirection = false;
        int cmd_indexes[MAXCMD];
        cmd_indexes[0] = 0;
        size_t arrIndex = 1;
        char* parallelPtr = NULL;
        char* outputFile[MAXCMD] = {NULL};
        while(token != NULL) {       
            if(strcmp(token, ">") == 0 && !redirection) { // redirection
                char* fileName = strtok(NULL, delim);
                if(!fileName) {
                    error();
                    err = true;
                } else {
                    outputFile[arrIndex-1] = strdup(fileName);
                }
                if(index == 0) {    // strtok(NULL, delim) || 
                    error();
                    err = true;
                }
                redirection = true;
                index--;
            } else if((redirPtr = strchr(token, '>')) != NULL && !redirection) { // redirection
                // int chIndex = redirPtr - cmd_argv[index];
                outputFile[arrIndex-1] = strdup(redirPtr + 1);   // output file
                *redirPtr = '\0';
                cmd_argv[index] = strdup(token);
                index++;
                if(!(outputFile[arrIndex-1])) { //  || strtok(NULL, delim)
                    error();
                    err = true;
                }
                redirection = true;
                index--;
            } else if(strcmp(token, "&") == 0) { // parallel commands
                cmd_argv[index] = NULL;
                cmd_indexes[arrIndex] = index+1;    // make sure when going through loop of cmd_indexes, ignore last index
                arrIndex++;
                redirection = false;
            } else if((parallelPtr = strchr(token, '&')) != NULL) { // parallel commands
                while(((parallelPtr = strchr(token, '&')) != NULL)) {
                    cmd_argv[index+2] = strdup(parallelPtr + 1);
                    cmd_argv[index+1] = NULL; 
                    cmd_indexes[arrIndex] = index+2;
                    arrIndex++;
                    *parallelPtr = '\0';
                    cmd_argv[index] = strdup(token);
                    index+=2;
                    token = cmd_argv[index];
                }
                redirection = false;
            } else {
                if(redirection) {
                    error();
                    err = true;
                    break;
                } else {
                    cmd_argv[index] = strdup(token);    // TODO do i have to free this later bc of strdup?
                    // strcpy(cmd_argv[index], token); // strcpy(dest, source);  // use strdup not strcpy otherwise issues with memory allocation (seg fault)
                }
            }
            // printf("cmd_argv[%ld]: %s\n", index, cmd_argv[index]);
            index++;
            // Subsequent calls: pass NULL to keep parsing the same string
            token = strtok(NULL, delim); 
            // token = strsep(&line, delim); 
        }
        cmd_argv[index] = NULL;
        
        free(line);

        if(cmd_argv[0] != NULL && !err) { // ensures there was an input (avoids seg fault if you just press enter)
            // built-in commands
            if(strcmp(cmd_argv[0], "exit") == 0) {
                if(cmd_argv[1] != NULL) {
                    error();
                } else {
                    if(stream) {
                        // if read from file
                        fclose(stream);
                    }
                    freeStrdup(pathDirs);
                    freeStrdup(cmd_argv);
                    exit(0);
                }
            } else if(strcmp(cmd_argv[0], "cd") == 0) {
                if(cmd_argv[2] != NULL || cmd_argv[1] == NULL
                    || (chdir(cmd_argv[1]) == -1)) {    // int chdir(const char *path);
                    // error (cd should only have two args total)
                    // return of -1 from chdir also indicates error
                    error();
                }
            } else if(strcmp(cmd_argv[0], "path") == 0) {
                freeStrdup(pathDirs);
                pathCount = 0;
                int i = 1;
                while(cmd_argv[i] != NULL) {
                    pathDirs[pathCount] = strdup(cmd_argv[i]);  // TODO test with cmd_argv[index] later to see if i need free()
                    pathCount++;
                    i++;
                }
                pathDirs[pathCount] = NULL;
            }

            // other commands 
            else {
                size_t iterations = arrIndex;
                for(size_t i = 0; i < arrIndex; i++) {
                    char** shifted_argv = &cmd_argv[cmd_indexes[i]]; // == cmd_argv + cmd_indexes[i];
                    if(shifted_argv[0] == NULL) {
                        // empty command
                        iterations--;
                        continue;
                    }


                    bool validCmd = false;
                    // char* path = "";
                    char path[1024];
                    //int snprintf(char *str, size_t size, const char *format, ...);
                    //int access(const char *pathname, int mode);
                    for(int i = 0; i < pathCount; i++) {
                        size_t pathCharLen = strlen(pathDirs[i]) + strlen(shifted_argv[0]) + 2; // +1 for '/' and +1 for \n
                        snprintf(path, pathCharLen, "%s/%s", pathDirs[i], shifted_argv[0]);
                        if(access(path, X_OK) == 0) {
                            // success
                            validCmd = true;
                            break;
                        }
                    }

                    if(validCmd) {
                        int rc = fork();

                        // creates new process
                        if(rc == 0) {
                            // child

                            // redirection
                            bool redirectionErr = false;
                            if(outputFile[i]) {
                                int fd = open(outputFile[i], O_WRONLY | O_CREAT | O_TRUNC, 0644); // int open(const char *pathname, int flags);
                                // fd is a file descriptor integer that refers to the open file (or -1 if error)
                                // flags: 
                                // O_WRONLY - write only access mode
                                // O_CREAT - creates output file (called outputFile) if it does not exist
                                // O_TRUNC - if file already exists, it will be truncated to length 0
                                // file permission:
                                // S_IRUSR - user has read permission
                                // S_IWUSR - user has write permission

                                if (fd == -1 || dup2(fd, STDOUT_FILENO) == -1) { //int dup2(int oldfd, int newfd);
                                    redirectionErr = true;
                                    error();
                                }
                            }

                            if(!redirectionErr) {
                                // execvp(cmd_argv[0], cmd_argv);
                                // execv(path, cmd_argv);
                                execv(path, shifted_argv);

                                // failed (error)
                                // if successful, doesn't return (aka doesn't print / run error())
                                error();
                                // printf("An error has occurred\n"); 
                            }
                        } 
                        // else if (rc > 0) {
                        //     // parent
                        //     // (void) wait(NULL);
                        // } // else failure
                    } else {
                        iterations--;
                        error();
                    }
                } 

                // parent
                for(size_t i = 0; i < iterations; i++) {
                    (void) wait(NULL);
                }

            }
        } 

        freeStrdup(cmd_argv);
        for(size_t i = 0; i < arrIndex; i++) {
            if(outputFile[i]) {
                free(outputFile[i]);
                outputFile[i] = NULL;
            }
        }
    }

    return 0;
}