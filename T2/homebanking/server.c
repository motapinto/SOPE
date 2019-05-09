#include "vetor.h"
#include "utilities.h"

sem_t sem_consumer_balcoes;
sem_t sem_producter_pedidos;
pthread_mutex_t exclusao_mutua;

static vetor *vec_global_acess = NULL;

pthread_t hash_balcao_livre(const pthread_t *vec_tids, const int dim);
int create_fifo_reply(pthread_t pid);
void *atendimento(void *request);
int op_check_balance(const int indice_vetor,const int fifo_fd_reply);
int check_authentication(const int n_conta, char *senha, int *indice_vetor_ret);

int server_initialization(pthread_t *vec_tids, const int n_threads_create, vetor *vec, int *fd_slog, char *senha_admin, int *fd_fifo)
{

    //Feito pelo Ruben:

    //1)Inicializar array dos tids

    for (int i = 0; i < n_threads_create; i++)
    {
        vec_tids[i] = ARRAY_TIDS_INICIALIZED; // inicializa a -1
    }

    //2)Criar Logfile para o server

    (*fd_slog) = open(SERVER_LOGFILE, O_CREAT | O_TRUNC | O_WRONLY, 0777);

    if (fd_slog < 0)
    {
        perror("Log file com erro: ");
        return -1;
    }

    //3)Inicializar os semaforo ao numero de threads

    logSyncMechSem((*fd_slog), MAIN_THREAD_ID, SYNC_OP_SEM_INIT, SYNC_ROLE_CONSUMER, UNKNOWN_REQUEST_PID, n_threads_create);

    sem_init(&sem_consumer_balcoes, 0, n_threads_create);

    logSyncMechSem((*fd_slog), MAIN_THREAD_ID, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, UNKNOWN_REQUEST_PID, 0);

    sem_init(&sem_producter_pedidos, 0, 0);

    //4)Inicializar o mutex

    if (pthread_mutex_init(&exclusao_mutua, NULL) != 0)
    {
        perror("Erro a inicializar o mutex_server_init:");
        return -1;
    }

    //5)Inicializar o array de clientes

    vec = vetor_novo();

    if (vec == NULL)
    {
        printf("Erro ao inicializar o vetor\n");
        return -1;
    }
    vec_global_acess = vec;

    //6)Criação da conta do administrador

    bank_account_t admin_init;

    admin_init.account_id = 0;
    admin_init.balance = 0;

    strcpy(admin_init.salt, salt_generator(senha_admin));
    strcpy(admin_init.hash, sha256sum(senha_admin, admin_init.salt));

    if (vetor_insere(vec, admin_init, 0) == -1)
    {
        printf("Conta do admin nao criada\n");
        return -1;
    }

    //7)Cria o FIFO "/tmp/secure_srv"

    mkfifo(SERVER_FIFO_PATH, 0777);

    //8)Abrir o fifo secure_srv em modo leitura

    (*fd_fifo) = open(SERVER_FIFO_PATH, O_CREAT | O_RDONLY | O_TRUNC | O_NONBLOCK, 0777); ////ABRI COM O_NONBLOCK, como e read_only,penso nao haver problemas

    if ((*fd_fifo) < 0)
    {
        perror("FIFO_SCR_SVR");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[], char *envp[])
{

    if (argc != 3)
    {
        printf("Incapaz de prosseguir execucao. Parametros do server errados\n");
        return -1;
    }

    int n_threads_create = atoi(argv[1]);
    int fd_slogfile, fd_fifo;

    pthread_t vec_tids[n_threads_create];

    vetor *bank_accounts = NULL;

    if (n_threads_create > MAX_BANK_OFFICES)
    {
        n_threads_create = MAX_BANK_OFFICES;
        printf("Foi introduzido um numero de balcoes superior, os balcoes foram truncados a %d:\n", MAX_BANK_OFFICES);
    }

    if (server_initialization(vec_tids, n_threads_create, bank_accounts, &fd_slogfile, argv[2], &fd_fifo) != 0)
    {
        printf("Server_init retornou com erro->Shutdown server\n");
        return -1;
    }

    printf("\n Execucao\n");

    //Execucao

    while (1)
    {

        //1)Ler pedidos do FIFO

        tlv_request_t leitura_aux;

        int ret_aux_value = read(fd_fifo, &leitura_aux, sizeof(leitura_aux));

        if (ret_aux_value < 0)
        {
            perror("Falhou a leitura do fifo secure_svr. Programa continua");
        }
        else if (ret_aux_value == 0)
        {
            //Nao le nada, como tal volta a tentar
            continue;
        }

        sem_post(&sem_producter_pedidos); //Recebi um pedido
        sem_wait(&sem_consumer_balcoes);  //Vou atribuir lhe balcao, decrementar "balcoes" disponiveis

        //Procuro um balcao livre
        pthread_t id_balcao_livre = hash_balcao_livre(vec_tids, n_threads_create);

        //Cria uma thread

        if (pthread_create(&id_balcao_livre, NULL, atendimento, &leitura_aux) != 0)
        {
            printf("Erro a criar thread\n");
        }
    }

    return 0;
}

pthread_t hash_balcao_livre(const pthread_t *vec_tids, const int dim)
{

    for (int i = 0; i < dim; i++)
    {

        if (vec_tids[i] == ARRAY_TIDS_INICIALIZED || vec_tids[i] == ARRAY_TIDS_FREE)
        {
            return i;
        }
    }

    printf("A hash da posicao nao encontrou balcoes livres. PANIC!!!!!! SINC PROBS\n");
    printf("PANIC\n");

    return 0;
}

int create_fifo_reply(pthread_t pid)
{

    //Utilitario para formular o fifo de resposta. Retorna um descritor onde podera ser escrito o TLV_ de resposta

    int fifo_fd = -1;
    char dir[200];
    char aux[100];

    strcpy(dir, USER_FIFO_PATH_PREFIX);

    sprintf(aux, "%lu", pid);

    strcat(dir, aux);

    mkfifo(dir, 0777);

    fifo_fd = open(dir, O_CREAT | O_TRUNC | O_WRONLY, 0777);

    if (fifo_fd < 0)
    {
        perror("Erro ao criar um FIFO de resposta");
    }

    return fifo_fd;
}

void *atendimento(void *request_ptr)
{
    tlv_request_t *ref = ((tlv_request_t *)request_ptr);

    int fifo_fd = -1;
    int type = ref->type;
    int indice_vetor;

    fifo_fd = create_fifo_reply(pthread_self());

    if (fifo_fd < 0)
    {
        printf("Erro no fifo de resposta a escrever resposta\n");
        return NULL;
    }


    if (check_authentication(ref->value.header.account_id, ref->value.header.password, &indice_vetor) != 0)
    {
        printf("Autenticacao falhada. Falta escrever bem no fifo\n");
        return NULL;
    }

    sem_wait(&sem_producter_pedidos); // Trato de 1 pedido
    pthread_detach(pthread_self());   // O porque do detached, se fizermos join adeus modelo concorrencial. Para alem disso vou escrever diretamente no FIFO de resposta

    switch (type)
    {
    case 0:
        //Criar Conta

        break;

    case 1:
        
        printf("Falta escrever para o log o envio de um consulta de saldo\n");
        
        if(op_check_balance(indice_vetor,fifo_fd)!=0){
            printf("Erro\n");
        }

        break;

    case 2:
        //FAZER TRANSFERENCIA

        break;

    case 3:
        //SERVER_SHUTDOWN

        break;

    default:
        printf("Erro a processar o atendimento. PANIC\n");
        return NULL;
    }

    printf("Falta escrever para o fifo a resposta");

    sem_post(&sem_consumer_balcoes); // Balcao fica livre

    
    return NULL;
}

int op_check_balance(const int indice_vetor,const int fifo_fd_reply)
{
    
    //Inicializacao da estrutura de resposta:
    tlv_reply_t resposta_consulta_saldo;

    resposta_consulta_saldo.type=OP_BALANCE;

    //
    printf("Temos de preparar a mensagem melhor em op check_balance\n");

    //Entrar em seccao critica de acesso ao vetor para consultar saldo

    printf("Registar o bloqueio do mutex\n");

    //S.C:

    pthread_mutex_lock(&exclusao_mutua);

    int saldo;
    
    saldo = vec_global_acess->elementos[indice_vetor].client->balance;
    
    resposta_consulta_saldo.value.balance.balance=saldo;

    resposta_consulta_saldo.length=sizeof(resposta_consulta_saldo)-(sizeof(resposta_consulta_saldo.type)+sizeof(resposta_consulta_saldo.length));
    
    
    printf("Falta registar que escrevi para o fifo\n");
    
    if(write(fifo_fd_reply,&resposta_consulta_saldo,sizeof(resposta_consulta_saldo))<0){
        perror("Erro na escrita para o fifo na consulta de saldo:");
        return -1;
    }

    if(pthread_mutex_unlock(&exclusao_mutua)!=0){
        perror("Erro no mutex, op: Consulta de Saldo:");
    }

    return 0;
}

int check_authentication(const int n_conta, char *senha, int *indice_vetor_ret)
{

    printf("Temos de preparar a mensagem melhor em check_authentication\n");

    char str_sal[SALT_LEN + 10];
    char str_hash[HASH_LEN + 10];

    if (senha == NULL)
    {
        printf("Senha invalida passada para check_authentic\n");
        return -1;
    }

    if (vec_global_acess == NULL)
    {
        printf("Incapaz de aceder ao vetor global\n");
        return -1;
    }

    //Primeiro vamos encontrar a conta com esse id da conta.
    int indice_vector = vetor_pesquisa(vec_global_acess, n_conta);

    (*indice_vetor_ret) = indice_vector; // Para evitar ter de percorrer o vetor_pesquisa outra vez;

    if (indice_vector == -2)
    {
        printf("Erro no vetor\n");
        return -2;
    }
    else if (indice_vector == -1)
    {
        printf("Conta nao encontrada. Temos de preparar a mensagem melhor\n");
        return -1;
    }

    //Encontrada a posicao, vamos verificar se a password e a correta

    strcpy(str_sal, vec_global_acess->elementos[indice_vector].client->salt);

    strcpy(str_hash, sha256sum(senha, str_sal));

    if (strcmp(str_hash, vec_global_acess->elementos[indice_vector].client->hash) == 0)
    {
        //Autenticacao validada
        return 0;
    }
    else
    {
        printf("Falta meter a autenticacao direita. Falhou validacao\n");
        return -1;
    }
}
