#include "utilities.h"

static const char alphanum[] ="0123456789!@#$%^&*ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

//Feito por Martim
char * salt_generator() 
{ 
    unsigned int rand_val;
    char salt_char;
    static char salt_str [SALT_LEN];
    static unsigned int counter = 0;

    srand(time(NULL) + counter);

    for(int i = 0; i < SALT_LEN; i++) {
        rand_val = rand() % strlen(alphanum) ;
        salt_char = alphanum[rand_val];

        salt_str[i] = salt_char;
    }
    counter ++;
    return salt_str;
}

//Feito por Martim
char *sha256sum(char *pass, char *salt)
{
    pid_t pid;
    int fd1[2], fd2[2];
    static char buffer[HASH_LEN + 1];

    if(pipe(fd1) != 0) {
        perror("pipe 1");
        exit(PIPE_ERROR);
    }

    if(pipe(fd2) != 0){
        perror("pipe 2");
        exit(PIPE_ERROR);
    }

    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(FORK_ERROR);
    }

    else if (pid > 0) {
        close(fd1[READ]);
        close(fd2[READ]);
        close(fd2[WRITE]);

        if(dup2(fd1[WRITE], STDOUT_FILENO) == -1){
            perror("dup2");
            exit(DUP_ERROR);
        }

        execlp("echo", "echo", "-n", NULL);

        close(fd1[WRITE]);
    }

    else {
        close(fd1[WRITE]);

        pid = fork();

        if (pid == -1) {
            perror("fork2");
            exit(1);
        }

        else if (pid > 0) { 
            /* Child 1 */
            close(fd2[READ]);

            if(dup2(fd1[READ], STDIN_FILENO) == -1){
                perror("dup2");
                exit(DUP_ERROR);
            }

            if(dup2(fd2[WRITE], STDOUT_FILENO) == -1){
                perror("dup2");
                exit(DUP_ERROR);
            }

            execlp("sha256sum", "sha256sum", NULL);

            close(fd2[WRITE]);
            close(fd1[READ]);
        }
        
        else {
            close(fd2[WRITE]);
            close(fd1[READ]);

            if(dup2(fd2[READ], STDIN_FILENO) == -1){
                perror("dup2");
                exit(DUP_ERROR);
            }

            read(fd2[READ], buffer, sizeof(buffer));
            close(fd2[READ]);
        }
    }

    return buffer;
}

//Feito por Martim
int ignoreSIGPIPE() 
{
    sigset_t sigmask;

    //Obter mascara atual
    if(sigprocmask(0, NULL, &sigmask) != 0) {
        perror("sigprocmask");
        exit(SIGPROCMASK_ERROR);
    }

    else {
        //Add to current mask SIGPIPE
        sigaddset(&sigmask, SIGPIPE); 
        //Set new mask
        if(sigprocmask(SIG_BLOCK, &sigmask, NULL) != 0) {
            perror("sigprocmask");
            exit(SIGPROCMASK_ERROR);
        }
    }

    return 0;
}
