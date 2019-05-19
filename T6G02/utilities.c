#include "utilities.h"

static const char alphanum[] = "0123456789abcdef";

//Feito por Martim
int salt_generator(char * salt_str)
{
    if(salt_str==NULL){
        
        return -1;
    }

    unsigned int rand_val;
    char salt_char;
    
    srand(time(NULL) + + pthread_self());

    for (int i = 0; i < SALT_LEN; i++) {
        rand_val = rand() % strlen(alphanum);
        salt_char = alphanum[rand_val];

        salt_str[i] = salt_char;
    }
    salt_str[SALT_LEN]='\0';
   
    return 0;
}

//Feito por Martim
int sha256sum(const char *pass, const char *salt, char*sha256sum_ret)
{
    if(pass==NULL||salt==NULL||sha256sum_ret==NULL){
        printf("Passei parametros a NULL no sha256sum\n");
        return -1;
    }
        
    int filedes_pipe_server_coprocesso[2];
    int filedes_pipe_copresso_server[2];
    pid_t fork_result;

    if(pipe(filedes_pipe_server_coprocesso)!=0){
        perror("Erro com o PIPE_envio no sha256sum:");
        return -1;
    }

    if(pipe(filedes_pipe_copresso_server)!=0){
        perror("Erro com o PIPE_resposta no sha256sum:");
        return -1;
    }


    fork_result=fork();

    if(fork_result<0){
        perror("Erro no fork do sha256sum:");
        return -1;
    }
    else if(fork_result>0){
        //PAI
        
        char str_cat_senha_sal[MAX_PASSWORD_LEN+SALT_LEN];
        strcpy(str_cat_senha_sal,pass);
        strcat(str_cat_senha_sal,salt);


        int return_filho;
          
        if(close(filedes_pipe_server_coprocesso[0])<0){
            perror("Erro no close 0:");
            return -1;
        }

        if(close(filedes_pipe_copresso_server[1])<0){
            perror("Erro no close 1:");
            return -1;
        }


        if(write(filedes_pipe_server_coprocesso[1],str_cat_senha_sal,strlen(pass)+strlen(salt))<0){
            perror("Erro no write do sha256sum:");
            return -1;
        }


        if(close(filedes_pipe_server_coprocesso[1])<0){
            perror("Erro no close 0:");
            return -1;
        }

        if(wait(&return_filho)==-1){
            perror("Erro no wait");
            return -1;
        }


        if(read(filedes_pipe_copresso_server[0],sha256sum_ret,HASH_LEN)<0){
            perror("Erro no read do sha256sum:");
            return -1;
        }
        
        //Verificacao adicional que o \0 foi colocado na string de retorno
        sha256sum_ret[HASH_LEN]='\0';

        if(close(filedes_pipe_copresso_server[0])<0){
            perror("Erro no close 5:");
        }

    }else if(fork_result==0){
        //FILHO

        //char buffer_recebido[MAX_PASSWORD_LEN+SALT_LEN];
        
        
        if(close(filedes_pipe_server_coprocesso[1])<0){
            perror("Erro no close 2:");
        }

        if(close(filedes_pipe_copresso_server[0])<0){
            perror("Erro no close 3:");
        }
    
        if(dup2(filedes_pipe_copresso_server[1],STDOUT_FILENO)<0){
            perror("Erro no dup2 do sha256sum:");
        }

        if(dup2(filedes_pipe_server_coprocesso[0],STDIN_FILENO)<0){
            perror("Erro no dup2 -2 do sha256sum:");
        }

        if(execlp("sha256sum","sha256sum",NULL)<0){
            perror("Erro no exec do sha256sum:");
        }

    }

    return 0;
}

int semaphore_operation(int fd_log, int id, sync_mech_op_t sem_operation, sync_role_t role, int sid, sem_t *semaphore_in_use, const int value_init_sem,pthread_mutex_t* escrita_log)
{

    int sem_value;

    if(semaphore_in_use==NULL||escrita_log==NULL){
        printf("Passei um semaforo ou mutex NULL para operation\n");
        return -1;
    }



    if(pthread_mutex_lock(escrita_log)!=0){
        perror("Erro no lock do mutex de escrita para o fdlog:");
        return -1;
    }


    if (sem_operation == SYNC_OP_SEM_INIT)
    {
        //Operation de inicializacao de semaforos

        if (logSyncMechSem(fd_log, id, SYNC_OP_SEM_INIT, role, sid, value_init_sem) < 0)
        {
            printf("Erro a registar um sem init no log file do server\n");
        }

        if (sem_init(semaphore_in_use, 0, value_init_sem) != 0)
        {
            perror("Erro a inicializar semaforo");
            return -1;
        }
    }

    else if (sem_operation == SYNC_OP_SEM_WAIT)
    {
        //Operation de Wait de semaforos
        sem_getvalue(semaphore_in_use,&sem_value);

        if (logSyncMechSem(fd_log, id, SYNC_OP_SEM_WAIT, role, sid,sem_value) < 0)
        {
            printf("Erro a registar um sem init no log file do server\n");
        }

        if(pthread_mutex_unlock(escrita_log)!=0){
            perror("Erro no lock do mutex de escrita para o fdlog:");
    
        }

        if (sem_wait(semaphore_in_use)!=0)
        {
            perror("Erro a fazer wait do semaforo");
            return -1;
        }
    }
    else if (sem_operation == SYNC_OP_SEM_POST)
    {
        //Operation de Post de semaforos
        
        if(sem_post(semaphore_in_use)!=0){
            perror("Erro a dar post ao semaforo:");
            return -1;
        }

        sem_getvalue(semaphore_in_use,&sem_value);

        if (logSyncMechSem(fd_log, id, SYNC_OP_SEM_POST, role, sid,sem_value) < 0)
        {
            printf("Erro a registar um sem init no log file do server\n");
        }

      
    }
    else
    {
        //Erro
        printf("Erro, na funcao de semaphore_operation. Atingi o else\n");
        perror("Erro nos semaforos:");
        return -1;
    }


    if(sem_operation!=SYNC_OP_SEM_WAIT){    
        //Evitar o undefined behaviour de dar unlock a mutexs unlocked e o dead lock provocado pelo wait
        
        if(pthread_mutex_unlock(escrita_log)!=0){
            perror("Erro no lock do mutex de escrita para o fdlog:");
            return -1;
        }
    }

    return 0;
}

int ignoreSIGPIPE()
{
    sigset_t sigmask;

    //Obter mascara atual
    if (sigprocmask(0, NULL, &sigmask) != 0) {
        perror("sigprocmask");
        exit(SIGPROCMASK_ERROR);
    }

    else {
        //Add to current mask SIGPIPE
        sigaddset(&sigmask, SIGPIPE);
        //Set new mask
        if (sigprocmask(SIG_BLOCK, &sigmask, NULL) != 0) {
            perror("sigprocmask");
            exit(SIGPROCMASK_ERROR);
        }
    }

    return 0;
}

int mutex_operation(int fd_log, int id, sync_mech_op_t mutex_operation, sync_role_t role, int sid, pthread_mutex_t *mutex_in_use,pthread_mutex_t* escrita_log)
{
    
    if(mutex_in_use==NULL||escrita_log==NULL){
        printf("Passamos um mutex NULL em mutex_operation\n");
        return -1;
    }
    
    
    if(pthread_mutex_lock(escrita_log)!=0){
        perror("Erro a dar lock ao mutex para escrita em log no mutex_operation\n");
        
    }

    if (mutex_operation == SYNC_OP_MUTEX_LOCK)
    {
        //Operation de Lock dos Mutexs

        if(pthread_mutex_lock(mutex_in_use)!=0){
            perror("Erro a dar lock ao mutex:");
            return -1;
        }

        if (logSyncMech(fd_log, id, mutex_operation, role, sid) < 0)
        {   printf("Erro a registar o mutex_lock no log file do server\n");
        }
    }


    else if (mutex_operation == SYNC_OP_MUTEX_UNLOCK)
    {
         if(pthread_mutex_unlock(mutex_in_use)!=0){
            perror("Erro a dar lock ao mutex:");
            return -1;
        }

        //lock

        if (logSyncMech(fd_log, id, mutex_operation, role, sid) < 0)
        {   printf("Erro a registar o mutex_lock no log file do server\n");
        }
    }

    if(pthread_mutex_unlock(escrita_log)!=0){
        perror("Erro a dar lock ao mutex para escrita em log no mutex_operation\n");
        return -1;
    }

    
    return 0;
}
