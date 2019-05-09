#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

const int BUFFER_SIZE = 100;
const int MAX_CMD_SIZE = 20;
const int MAX_CMD_N = 20;

int main(int argc, char * argv[]) {

    pid_t pid;
    char line[BUFFER_SIZE];
    char * cmd[MAX_CMD_N];
    int curr = 0;
    int simple = 1;

    printf("minish >");
    fgets(line, BUFFER_SIZE, stdin);

    /* Remove newline */
    int length = strlen(line);
    if (line[length - 1] == '\n') 
        line[length - 1] = '\0';

    /* No flags */
    if (strchr(line, ' ') == NULL) {
        cmd[0] = line;
    }
    /* Has flags */
    else {
        char * buf;
        simple = 0;

        buf = strtok(line, " ");
        while(buf != NULL) {
            cmd[curr++] = buf;
            buf = strtok(NULL, " ");
        }
        cmd[curr] = NULL;
    }

    while(strcmp(cmd[0], "quit") != 0) {

        pid = fork();
        /* error */
        if (pid == -1) {
            perror("Fork");
            exit(1);
        }
        /* parent */
        if (pid > 0) {
            int status;
            wait(&status);
        }
        /* child */
        else {
            if (simple) {
                execlp(cmd[0], cmd[0], NULL);
            }
            else {
                /* check for -o <file> */
                if (curr > 2 && strcmp(cmd[curr-2], "-o") == 0) {
                    FILE * out = fopen(cmd[curr-1], "w");
                    dup2(fileno(out), STDOUT_FILENO);
                    cmd[curr-2] = NULL;
                    execvp(cmd[0], cmd);
                }
                else {
                    execvp(cmd[0], cmd);
                }
            }
            printf("Error executing execlp\n");
            exit(1);
        }

        printf("\nminish >");
        fgets(line, BUFFER_SIZE, stdin);

        /* Remove newline */
        int length = strlen(line);
        if (line[length - 1] == '\n')
            line[length - 1] = '\0';

        /* No flags */
        if (strchr(line, ' ') == NULL) {
            cmd[0] = line;
            simple = 1;
        }
        /* Has flags */
        else {
            char * buf;
            curr = 0;
            simple = 0;

            buf = strtok(line, " ");
            while(buf != NULL) {
                cmd[curr++] = buf;
                buf = strtok(NULL, " ");
            }
            cmd[curr] = NULL;
        }
    }

    return 0;
}