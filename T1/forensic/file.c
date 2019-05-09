#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include "file.h"
#include <dirent.h>
#include "utilities.h"
#include <sys/stat.h>
#include <time.h>
#include "signals.h"
#include <signal.h>
#include <sys/time.h>

bool output = false, logs = false, md5 = false, sha1 = false, sha256 = false;
FILE *fp_logs = NULL;
char *file_name_glob ;

char *return_date(char *buffer)
{
    static char date[50];
    char modify_date[50], *token_init, *token_final;
    char *new_buf;

    struct tm ts;
    struct stat leitura_info;

    stat(buffer, &leitura_info);

    ts = *localtime(&leitura_info.st_ctime);

    char buf[50];
    strftime(buf, sizeof(buf), "%Y-%M:%S, ", &ts);

    fflush(stdout);

    new_buf = strstr(buffer, "Access");
    new_buf = strstr(new_buf, "Access");
    new_buf = strstr(new_buf, "2");

    strncpy(date, new_buf, 19);
    date[10] = 'T';

    new_buf = strstr(new_buf, "Modify");
    new_buf = strstr(new_buf, "2");

    strncpy(modify_date, new_buf, 19);
    modify_date[19] = '\0';
    modify_date[10] = 'T';

    strcat(date, ", ");
    strcat(date, modify_date);

    return date;
}

void print_event_type(char *str)
{
    char *str_log = (char *)malloc(sizeof(char) * 500);

    if (fp_logs != NULL && logs == true)
    {
        char buffer_time[50];
        char buf_aux[50];
        struct tm *tm_info;
        struct timeval det_tm;
        time_t timer;
        
        time(&timer);
        tm_info = localtime(&timer);
        strftime(buffer_time, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        gettimeofday(&det_tm, NULL);
        
        sprintf(buf_aux, ":%ld ", det_tm.tv_usec / 10000);

        strcat(buffer_time,buf_aux);


        strcpy(str_log, buffer_time); //copia o current time
        
        pid_t pid = getpid();
        sprintf(buf_aux, "- %d - ", pid);
        
        strcat(str_log,buf_aux);
        
        
        if (str != NULL)
        {
            strcat(str_log, str);
            fprintf(fp_logs, "%s\n", str_log);
            
        }

    }
    if (fp_logs != NULL)
    {
        fprintf(fp_logs,"%s\n",str_log);
        fflush(fp_logs);
    }
    else
    {
        printf("Erro no logs\n");
    }

    free(str_log);
    return;
}

void file_info(FILE *fp, char *file_name)
{
    if (output)
    {
        if (is_directory(file_name))
        {
            raise(SIGUSR1);
            print_event_type("Signal SIGUSR1");
        }
        else
        {
            (raise(SIGUSR2));
            print_event_type("Signal SIGUSR2");
        }
    }

    fprintf(fp, "%s, ", file_info_file(file_name));
    fprintf(fp, "%d, ", file_info_size(file_name));
    fprintf(fp, "%s, ", file_info_permissions(file_name));
    fprintf(fp, "%s, ", file_info_date(file_name));

    if (is_directory(file_name) == false)
    {
        print_cripto_sum(file_name, fp);
    }

    fprintf(fp, "\n");
    return;
}

int command_interpreter(int argc, char *argv[], char *envp[])
{
    signal(SIGINT, sig_handler);
    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);
    int pos = 0, fd, envp_pos = 0, fd_logs;
    char *dest_file_name, *file_name = argv[argc - 1], *envp_file_name; // file_name é o ultimo argumento argv

    output = false, md5 = false, sha1 = false, sha256 = false;
    bool recursive = false;
    //           -o                -h                                    -r

    FILE *fp = stdout;
    file_name_glob = file_name;
    int parent_pid = getpid();

    while (argv[pos] != NULL)
    {

        if (strcmp(argv[pos], "-r") == 0)
        {
            recursive = true;
        }

        if (strcmp(argv[pos], "-h") == 0)
        {
            pos++;
            char *token = strtok(argv[pos], ",");

            while (token != NULL)
            {

                if (strcmp(token, "md5") == 0)
                    md5 = true;
                if (strcmp(token, "sha1") == 0)
                    sha1 = true;
                if (strcmp(token, "sha256") == 0)
                    sha256 = true;
                token = strtok(NULL, ",");
            }
        }

        if (strcmp(argv[pos], "-o") == 0)
        {
            // se tiver um -o tenho de ler o nome do ficheiro de output que substitui a stdout
            pos++;

            if (argc >= pos)
            { // se houve argc's á direita. dupla - verificação
                dest_file_name = argv[pos];
                output = true;
            }
        }

        if (strcmp(argv[pos], "-v") == 0)
        {
            logs = true;
        }

        pos++;
    }

    if (output)
    {

        if ((fd = open(dest_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
        { //opens destination file

            perror("Can't open destination file");
            exit(2);
        }
        if ((fp = fdopen(fd, "w")) == NULL)
        { //file pointer now points to output file
            perror("Can't open destination file");
            exit(2);
        }
    }

    if (logs)
    {
        envp_pos = search_envp(envp, "LOGNAME"); //LOGNAME OR LOGNAMEFILE
        envp_file_name = strstr(envp[envp_pos], "=");
        envp_file_name++;
        strcat(envp_file_name, ".txt"); //este vai ser o ficheiro utilizado para os logs

        fp_logs=fopen(envp_file_name,"a");
        
        if(fp_logs==NULL)
        {
            perror("Can't open logs file");
            exit(3);
        }
    }

    if (recursive && is_directory(file_name))
    {
        recursive_call(file_name, argc, argv, envp, fp);
        while(wait(NULL) > 0);
    }
    else
    {
        file_info(fp, file_name);
    }

    if (output)
    {
        fflush(fp); 
        fclose(fp);

        if(getpid() == parent_pid) {
            fprintf(stdout, "Data saved on file output.txt\n");
            fprintf(stdout, "Execution records saved on file ...\n");
        }
    }

    if (logs)
    {
        if(fp_logs!=NULL){
            print_event_type("New file found");
            fflush(fp_logs);
        }

        fclose(fp_logs);
    }

    return 0;
}

void recursive_call(char *file_name, int argc, char *argv[], char *envp[], FILE *fp)
{
    int i = 0;
    int file_counter;

    char *files;
    char *new_file_name = (char *)calloc(6000, sizeof(char));
    char aux[10000];

    DIR *New_dir = NULL;

    if (is_directory(file_name))
    {
        files = folder_list_files(file_name);
    }

    files = strtok(files, ",");

    file_info(fp, file_name);

    while (files != NULL)
    {
        if (i == 0)
        {
            i++;
        }
        else
        {
            files++;
        }

        strcpy(new_file_name, file_name);

        strcat(new_file_name, "/");
        strcat(new_file_name, files);

        if (new_file_name[strlen(new_file_name) - 1] == '\n')
        {

            new_file_name[strlen(new_file_name) - 1] = '\0';
        }

        if ((is_directory(new_file_name) == false))
        {
            file_info(fp, new_file_name);
        }
        else if ((is_directory(new_file_name) == true))
        {
            __pid_t pid;

            strcpy(aux, argv[argc - 1]);

            strcpy(argv[argc - 1], new_file_name);

            pid = fork();

            if (pid == 0)
            {
                recursive_call(new_file_name, argc, argv, envp, fp);
                while((wait(NULL)) > 0);
            }
            else
            {
                while((wait(NULL)) > 0);
                strcpy(argv[argc - 1], aux);
            }
        }

        files = strtok(NULL, ",");
    }
    free(new_file_name);

    while((wait(NULL)) > 0);

    return;
}

void print_cripto_sum(char *file_name, FILE *fp)
{

    if (md5)
    { //--------------------------------------------------------------------
        fprintf(fp, "%s, ", file_info_md5sum(file_name));
        fflush(fp); //garante que é escrito no output file depois de fprints
    }
    if (sha1)
    {
        fprintf(fp, "%s, ", file_info_sha1sum(file_name));
        fflush(fp);
    }
    if (sha256)
    {
        fprintf(fp, "%s, ", file_info_sha256sum(file_name));
        fflush(fp);
    }
    return;
}

int search_envp(char *envp[], char *var)
{
    int pos = 0;
    while (envp[pos] != NULL)
    {
        if (strstr(envp[pos], var) != 0)
        {
            break;
        }
        pos++;
    }
    return pos;
}

bool is_directory(char *file_name) //--------------------------------------------------------
{
    struct stat buf;
    stat(file_name, &buf);

    return !S_ISREG(buf.st_mode); //retorna se for diretorio, true.
}

char *file_info_date(char *fileName)
{
    struct tm *ta, *tm;
    struct stat leitura_info;

    char *buf_a = (char *)calloc(105, sizeof(char));
    char *buf_m = (char *)calloc(105, sizeof(char));

    stat(fileName, &leitura_info);

    ta = localtime(&leitura_info.st_atime);
    strftime(buf_a, 99, "%Y-%m-%dT%H:%M:%S, ", ta);

    tm = localtime(&leitura_info.st_ctime);
    strftime(buf_m, 99, "%Y-%m-%dT%H:%M:%S", tm);

    buf_a = realloc(buf_a, strlen(buf_a)+strlen(buf_m));
    strcat(buf_a, buf_m);

    return buf_a;
}

char *file_info_file(char *fileName)
{
    int filedes[2];

    if (pipe(filedes) == -1)
    {
        perror("Pipe");
        exit(1);
    }
    pid_t pid = fork(); // Pai e filho ficam a comunicar por pipe
    if (pid == -1)
    {
        perror("Fork");
        exit(1);
    }
    else if (pid == 0)
    {
        while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR))
        {
        } // dup2 redireciona o stdout do filho para o canal de escrita do pipe

        //abre e fecha o canal a cada chamada da função
        close(filedes[0]);
        //close(filedes[1]);  !!??Nao vale a pena fazer este close, pois quando se o processo termina (com exec) as entradas da tabela de escritores fecha todas as entradas
        execlp("file", "file", fileName, NULL);
        perror("Execlp");
        exit(1);
    }

    //Processo - pai começa aqui:
    close(filedes[1]); //faz close porque nao vai escrever nada

    char *buffer = calloc(20000, sizeof(char));

    while (1)
    {
        ssize_t count = read(filedes[0], buffer, 19999);
        if (count == -1)
        {
            if (errno == EINTR) // A escrita é interrompida por um sinal. Tentar ler novamente
            {
                continue;
            }
            else
            {
                perror("Read");
                exit(1);
            }
        }
        else if (count == 0)
        {
            break;
        }
        else
        {
            // printf("VER ISTO\n");
            (buffer, count); // ??
        }
    }
    close(filedes[0]);

    int status;

    wait(&status); // valor do retorno do filho

    if (status != 0)
    {
        printf("Filho retorna com erro.\n");
        exit(1);
    }

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = '\0'; //no fim da linha substituimos o \n por \0 para evitar que haja mudança de linha
    // free(buffer);

    buffer = realloc(buffer, strlen(buffer) + 10);

    return buffer;
}

int file_info_size(char *fileName)
{
    struct stat buf;
    stat(fileName, &buf);
    return buf.st_size;
}

char *file_info_permissions(char *fileName)
{
    struct stat buf;
    int permissions;

    stat(fileName, &buf);

    permissions = buf.st_mode;
    permissions %= 010000;    //separates octal number to keep just permissions
    permissions /= pow(8, 2); //keeps first set of permission (user)

    return octal_to_string_file_premission(permissions);
}

char *file_info_md5sum(char *fileName)
{
    int filedes[2];
    if (pipe(filedes) == -1)
    {
        perror("Pipe");
        exit(1);
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Fork");
        exit(1);
    }
    else if (pid == 0)
    {
        while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR))
        {
        }
        close(filedes[1]);
        close(filedes[0]);
        execlp("md5sum", "md5sum", fileName, NULL);
        perror("Execlp");
        exit(1);
    }
    close(filedes[1]);
    static char buffer[100];
    while (1)
    {
        ssize_t count = read(filedes[0], buffer, sizeof(buffer));
        if (count == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("Read");
                exit(1);
            }
        }
        else if (count == 0)
        {
            break;
        }
        else
        {
            (buffer, count);
        }
    }
    close(filedes[0]);

    int status;
    wait(&status); // valor do retorno do filho

    if (status != 0)
        printf("Filho retorna com erro.\n");

    char *p = strchr((char *)buffer, ' ');
    if (!p)
    {
        perror("Strchr");
    }
    else
        *p = 0;

    return buffer;
}

char *file_info_sha1sum(char *fileName)
{
    int filedes[2];
    if (pipe(filedes) == -1)
    {
        perror("Pipe");
        exit(1);
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Fork");
        exit(1);
    }
    else if (pid == 0)
    {
        while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR))
        {
        }
        close(filedes[1]);
        close(filedes[0]);
        execlp("sha1sum", "sha1sum", fileName, NULL);
        perror("Execlp");
        exit(1);
    }
    close(filedes[1]);
    static char buffer[100];
    while (1)
    {
        ssize_t count = read(filedes[0], buffer, sizeof(buffer));
        if (count == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("Read");
                exit(1);
            }
        }
        else if (count == 0)
        {
            break;
        }
        else
        {
            (buffer, count);
        }
    }
    close(filedes[0]);

    int status;
    wait(&status); // valor do retorno do filho

    if (status != 0)
        printf("Filho retorna com erro.\n");

    char *p = strchr((char *)buffer, ' ');
    if (!p)
    {
        perror("Strchr");
    }
    else
        *p = 0;
    return buffer;
}

char *file_info_sha256sum(char *fileName)
{
    int filedes[2];
    if (pipe(filedes) == -1)
    {
        perror("Pipe");
        exit(1);
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Fork");
        exit(1);
    }
    else if (pid == 0)
    {
        while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR))
        {
        }
        close(filedes[1]);
        close(filedes[0]);
        execlp("sha256sum", "sha256sum", fileName, NULL);
        perror("Execlp");
        exit(1);
    }
    close(filedes[1]);
    static char buffer[100];
    while (1)
    {
        ssize_t count = read(filedes[0], buffer, sizeof(buffer));
        if (count == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("Read");
                exit(1);
            }
        }
        else if (count == 0)
        {
            break;
        }
        else
        {
            (buffer, count);
        }
    }
    close(filedes[0]);

    int status;
    wait(&status); // valor do retorno do filho

    if (status != 0)
        printf("Filho retorna com erro.\n");

    char *p = strchr((char *)buffer, ' ');
    if (!p)
    {
        perror("Strchr");
    }
    else
        *p = 0;
    return buffer;
}

char *folder_list_files(char *fileName)
{
    int filedes[2];

    if (pipe(filedes) == -1)
    {
        perror("Pipe");
        exit(1);
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Fork");
        exit(1);
    }

    else if (pid == 0)
    { //filho
        while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR))
        {
        }

        close(filedes[0]);
        execlp("ls", "ls", "-m", fileName, NULL);
        perror("Execlp");

        exit(1);
    }

    close(filedes[1]);

    char *buffer = (char *)calloc(100000, sizeof(char));

    while (1)
    {
        ssize_t count = read(filedes[0], buffer, 9990);
        if (count == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("Read");
                exit(1);
            }
        }
        else if (count == 0)
        {
            break;
        }
        else
        {
            (buffer, count);
        }
    }
    close(filedes[0]);

    buffer = realloc(buffer, strlen(buffer) + 10);

    return buffer;
}