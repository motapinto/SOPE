#include "queue.h"
#include "utilities.h"
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include "constants.h"

typedef struct
{

    bank_account_t conta_info;
    pthread_mutex_t mutex_acesso_cont;
    int initialized;

} bank_account;

typedef struct
{
    bank_account *vec_bank_accounts;
    int n_balcao;
} mensage_to_thread;

sem_t sem_consumer_balcoes;
sem_t sem_producter_pedidos;
pthread_mutex_t acesso_fila;
pthread_mutex_t escrita_logs;
pthread_cond_t condition_var_shutdown;
pthread_cond_t condition_var_transfer;
static fila *fila_pedidos_waiting = NULL;
static int fd_slog;
static bool block_request = false;
static bool shutdown_req_active = false;

int server_initialization(pthread_t *vec_tids, int *vec_tids_indices, const int n_threads_create, char *senha_admin, bank_account *vec_contas);

void *atendimento(void *arg);

int op_create_account(const uint32_t account_id_request, char *password_request, const uint32_t account_id_create, char *password_create, const uint32_t balance_inicial, const int balcao_caller, const useconds_t op_delay_ms, int *return_code, const int fifo_fd_reply, bank_account *vec_bank_accounts);
int op_check_balance(const int account_id, char *senha, const int fifo_fd_reply, const useconds_t op_delay_ms, int return_code, const int balcao_caller, bank_account *vec_bank_accounts);
int op_make_transfer(const uint32_t account_id_origem, char *senha, const uint32_t account_id_destino, const uint32_t valor_transfer, const int fifo_fd_reply, const useconds_t op_delay_ms, const int32_t n_balcao, int *return_code, bank_account *vec_bank_accounts);
int op_server_shutdown(const int account_id, char *senha, const int fifo_fd_reply, const useconds_t op_delay_ms, int *return_code, const int balcao_caller, const int pid_request_id, bank_account *vec_bank_accounts);

int open_fifo_reply(pid_t pid, int *return_code, int *fifo_fd);
int write_reply(const int fifo_fd_reply, int return_code, const int account_id_create, const int n_balcao, const int operation_type_to_return);
int check_authentication(const int n_conta, char *senha, const bank_account *vec_bank_accounts);

int server_initialization(pthread_t *vec_tids, int *vec_tids_indices, const int n_threads_create, char *senha_admin, bank_account *vec_contas)
{
    //Feito pelo Ruben:

    if (vec_tids == NULL || vec_tids_indices == NULL || senha_admin == NULL)
    {
        printf("Algo esta a NULL na inicializacao\n");
        return -1;
    }

    //1)Criar Logfile para o server

    fd_slog = open(SERVER_LOGFILE, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    if (fd_slog < 0)
    {
        perror("Log file com erro: ");
        return -1;
    }

    //2)Inicializar queue dos pedidos

    fila_pedidos_waiting = fila_nova();
    if (fila_pedidos_waiting == NULL)
    {
        printf("Erro a criar a fila\n");
        return -1;
    }
    //3)Inicializar os semaforo ao numero de threads

    if (semaphore_operation(fd_slog, MAIN_THREAD_ID, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, UNKNOWN_REQUEST_PID, &sem_consumer_balcoes, n_threads_create, &escrita_logs) != 0)
    {
        printf("Erro em semaphore_operation na inicializacao\n");
        return -1;
    }

    if (semaphore_operation(fd_slog, MAIN_THREAD_ID, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, UNKNOWN_REQUEST_PID, &sem_producter_pedidos, 0, &escrita_logs) != 0)
    {
        printf("Erro em semaphore_operation na inicializacao\n");
        return -1;
    }

    //4)Criação da conta do administrador
    tlv_request_t admin_account;

    admin_account.length = sizeof(admin_account.value.header);
    ;
    admin_account.type = OP_CREATE_ACCOUNT;
    admin_account.value.create.account_id = ADMIN_ACCOUNT_ID;
    admin_account.value.create.balance = 0;
    admin_account.value.header.op_delay_ms = 0;
    admin_account.value.header.account_id = 0;
    strcpy(admin_account.value.create.password, senha_admin);

    if (fila_push(fila_pedidos_waiting, &admin_account) != 1)
    {
        printf("Falhou a colocar o pedido de criacao de conta do admin\n");
        return -1;
    }

    //5)Inicializar os mutexs

    for (int i = 0; i < MAX_BANK_ACCOUNTS; i++)
    {

        if (pthread_mutex_init(&vec_contas[i].mutex_acesso_cont, NULL) != 0)
        {
            perror("Erro a inicializar o mutex_server_init:");
            return -1;
        }

        vec_contas[i].initialized = ACCOUNT_ARRAY_POS_NOT_INITIALIZED;
    }

    if (pthread_mutex_init(&acesso_fila, NULL) != 0)
    {
        perror("Erro a inicializar o mutex_server_init:");
        return -1;
    }

    if (pthread_mutex_init(&escrita_logs, NULL) != 0)
    {
        perror("Erro a inicializar o mutex_log_write:");
        return -1;
    }

    //6)Inicializa as variaveis de condicao

    if (pthread_cond_init(&condition_var_shutdown, NULL) != 0)
    {
        perror("Erro a dar init as vars de condicao em server_init");
        return -1;
    }
    if (pthread_cond_init(&condition_var_transfer, NULL) != 0)
    {
        perror("Erro a dar init as vars de condicao em server_init");
        return -1;
    }

    //7)Inicializa o array de contas

    //8)Inicializa o vetor dos indices dos tids

    for (int i = 0; i < n_threads_create; i++)
    {
        vec_tids_indices[i] = i + 1;
    }

    //9)Faz ignore a SIGPIPES, tal como pretendido no enunciado
    if (ignoreSIGPIPE() != 0)
    {
        fprintf(stderr, "Unbale to ignore SIGPIPE");
        return IGNORE_SIGPIPE_ERROR;
    }

    return 0;
}

int main(int argc, char *argv[], char *envp[])
{
    int n_threads_create = atoi(argv[1]);
    pthread_t vec_tids[n_threads_create];
    int vec_tids_indices[n_threads_create];
    bank_account vec_bank_accounts[MAX_BANK_ACCOUNTS];
    mensage_to_thread mensagem_thread_aux[MAX_BANK_ACCOUNTS];

    if (argc != 3)
    {
        printf("Incapaz de prosseguir execucao. Parametros do server errados\n");
        return -1;
    }

    //Trunca o n de threads
    if (n_threads_create > MAX_BANK_OFFICES)
    {
        n_threads_create = MAX_BANK_OFFICES;
        printf("Foi introduzido um numero de balcoes superior, os balcoes foram truncados a %d:\n", MAX_BANK_OFFICES);
    }

    if (server_initialization(vec_tids, vec_tids_indices, n_threads_create, argv[2], vec_bank_accounts) != 0)
    {
        printf("Server_init retornou com erro->Shutdown server\n");
        return -1;
    }

    //Crio os threads

    for (int i = 0; i < n_threads_create; i++)
    {
        mensagem_thread_aux[i].vec_bank_accounts = vec_bank_accounts;
        mensagem_thread_aux[i].n_balcao = i;

        if (pthread_create(&vec_tids[i], NULL, atendimento, &mensagem_thread_aux[i]) != 0)
        {
            perror("Erro a criar thread");
        }

        if (logBankOfficeOpen(fd_slog, i + 1, vec_tids[i]) < 0)
        {
            printf("Erro no logbankofficeope\n");
        }
    }

    if (semaphore_operation(fd_slog, MAIN_THREAD_ID, SYNC_OP_SEM_WAIT, SYNC_ROLE_PRODUCER, ADMIN_ACCOUNT_ID, &sem_consumer_balcoes, PARAMETER_NOT_USED, &escrita_logs) != 0)
    {
        printf("Erro a fazer post do pedido em server init\n");
        return -1;
    }

    if (semaphore_operation(fd_slog, MAIN_THREAD_ID, SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, ADMIN_ACCOUNT_ID, &sem_producter_pedidos, PARAMETER_NOT_USED, &escrita_logs) != 0)
    {
        printf("Erro a fazer post do pedido em server init\n");
        return -1;
    }

    //6)Cria o FIFO "/tmp/secure_srv"

    int fd_fifo_srv = -1;

    if (mkfifo(SERVER_FIFO_PATH, 0777) < 0 && errno != EEXIST)
    {
        perror("Erro a fazer mkfifo no secure_srv:");
        return -1;
    }

    fd_fifo_srv = open(SERVER_FIFO_PATH, O_RDONLY); //abrir fifo scr_svr

    if (fd_fifo_srv < 0)
    {
        perror("FIFO_SCR_SVR:");
        return -1;
    }

    //Execucao

    while (1)
    {
        //1)Ler pedidos do FIFO
        tlv_request_t leitura_aux;
        int read_ret_value = -1;

        if (shutdown_req_active == true)
        {
            //Incrementa um pedido que estava parado no FIFO quando chegou um shutdown request
            //Momento de parar o ciclo, desbloquear o thread que esta a processar o encerramento e terminar o while. Fazer join e tomar todas as precaucoes de encerramento do programa
            if (fila_tamanho(fila_pedidos_waiting) == 0)
            {
                /////Escreve para o log o cond_wait
                if (pthread_mutex_lock(&escrita_logs) != 0)
                {
                    perror("Erro no lock do mutex de escrita para o fdlog:");
                }

                if (logSyncMech(fd_slog, MAIN_THREAD_ID, SYNC_OP_COND_SIGNAL, SYNC_ROLE_PRODUCER, leitura_aux.value.header.pid) < 0)
                {
                    printf("Erro a escrever para o log do server o delay em op_server_shutdown\n");
                }

                if (pthread_mutex_unlock(&escrita_logs) != 0)
                {
                    perror("Erro no unlock do mutex de escrita para o fdlog:");
                }

                ///Envia o signal que desbloqueia o thread
                if (pthread_cond_signal(&condition_var_shutdown) != 0)
                {
                    perror("No main a dar condsignal");
                }

                break;
            }
        }

        read_ret_value = read(fd_fifo_srv, &leitura_aux, sizeof(tlv_request_t));

        if (read_ret_value < 0)
        {
            perror("Falhou leitura:");
        }

        else if (read_ret_value == 0)
        {
            continue;
        }

        if (pthread_mutex_lock(&escrita_logs) != 0)
        {
            perror("Erro no lock do mutex de escrita para o fdlog:");
        }

        if (logRequest(fd_slog, MAIN_THREAD_ID, &leitura_aux) < 0)
        {
            printf("Erro a escrever no logrequest no main\n");
        }

        if (pthread_mutex_unlock(&escrita_logs) != 0)
        {
            perror("Erro no lock do mutex de escrita para o fdlog:");
        }

        if (semaphore_operation(fd_slog, MAIN_THREAD_ID, SYNC_OP_SEM_WAIT, SYNC_ROLE_PRODUCER, leitura_aux.value.header.pid, &sem_consumer_balcoes, PARAMETER_NOT_USED, &escrita_logs) < 0)
        {
            printf("Erro no logsyncmechsem0\n");
            return -1;
        }

        if (mutex_operation(fd_slog, MAIN_THREAD_ID, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, leitura_aux.value.header.pid, &acesso_fila, &escrita_logs) != 0)
        {
            printf("Erro a dar lock do mutex para aceder a fila em main\n");
            return -1;
        }

        if (fila_push(fila_pedidos_waiting, &leitura_aux) != 1 && block_request == false)
        {
            printf("Erro a inserir na fila de pedidos\n");
        }

        if (leitura_aux.type == OP_SHUTDOWN && shutdown_req_active == false)
        {
            //Mal recebe o pedido de shutdown a variavel passa a valer o n pedidos na fila. Outros que entrem, passaram a ser incrementados
            fila_pedidos_waiting->tamanho_maximo_apos_shutdown = fila_tamanho(fila_pedidos_waiting);

            shutdown_req_active = true;
        }

        if (mutex_operation(fd_slog, MAIN_THREAD_ID, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, leitura_aux.value.header.pid, &acesso_fila, &escrita_logs) != 0)
        {
            printf("Erro a dar lock do mutex para aceder a fila em main\n");
            return -1;
        }

        if (semaphore_operation(fd_slog, MAIN_THREAD_ID, SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, leitura_aux.value.header.pid, &sem_producter_pedidos, PARAMETER_NOT_USED, &escrita_logs) < 0)
        {
            printf("Erro a dar post no main\n");
            return -1;
        }
    }

    for (int i = 0; i < n_threads_create; i++)
    {

        if (semaphore_operation(fd_slog, MAIN_THREAD_ID, SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, UNKNOWN_REQUEST_PID, &sem_producter_pedidos, PARAMETER_NOT_USED, &escrita_logs) != 0)
        {
            printf("Erro a fazer wait no shutdown\n");
            return -1;
        }
    }

    // Encerramento Threads
    for (int i = 0; i < n_threads_create; i++)
    {
        if (pthread_join(vec_tids[i], NULL) != 0)
        {
            perror("Erro pthread_join no main");
        }
    }

    if (close(fd_fifo_srv) != 0)
    {
        perror("Erro a dar close de fd_fifo_srv:");
    }
    if (close(fd_slog) != 0)
    {
        perror("Erro a dar close de fd_slog:");
    }

    if (unlink(SERVER_FIFO_PATH) != 0)
    {
        perror("Erro a fazer unlink do fifo_srv:");
        return -1;
    }

    fila_apaga(fila_pedidos_waiting);

    return 0;
}
////////////////////////////////////////
void *atendimento(void *arg)
{
    int fd_fifo_reply_fd = -1;
    tlv_request_t *ref;
    tlv_request_t variavel_armazenamento;
    mensage_to_thread *pointer_ref;
    bank_account *vec_bank_accounts;

    pointer_ref = (mensage_to_thread *)arg;

    mensage_to_thread thread_info_receive;

    thread_info_receive.vec_bank_accounts = pointer_ref->vec_bank_accounts;
    thread_info_receive.n_balcao = pointer_ref->n_balcao;

    int n_balcao = thread_info_receive.n_balcao;
    vec_bank_accounts = thread_info_receive.vec_bank_accounts;

    //Balcao em atendimento.

    //printf("Pensar melhor forma de matar os threads");

    while (1)
    {
        int return_code = RET_CODE_INITIALIZATION;

        if (semaphore_operation(fd_slog, n_balcao, SYNC_OP_SEM_WAIT, SYNC_ROLE_CONSUMER, UNKNOWN_REQUEST_PID, &sem_producter_pedidos, PARAMETER_NOT_USED, &escrita_logs) != 0)
        {
            printf("Erro a fazer wait no atendimento\n");
            return NULL;
        }
        //Entra em competicao

        if (shutdown_req_active == true && fila_tamanho(fila_pedidos_waiting) == 0)
        {
            if (pthread_mutex_lock(&escrita_logs) != 0)
            {
                perror("Erro no lock do mutex de escrita para o fdlog:");
            }

            if (logBankOfficeClose(fd_slog, n_balcao, pthread_self()) < 0)
            {
                printf("Erro a escrever para o log do server o delay em op_check_balance\n");
            }

            if (pthread_mutex_unlock(&escrita_logs) != 0)
            {
                perror("Erro no unlock do mutex de escrita para o fdlog:");
            }

            pthread_exit(NULL);
        }

        //Acesso em S.C a fila
        if (mutex_operation(fd_slog, n_balcao, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, n_balcao, &acesso_fila, &escrita_logs) != 0)
        {
            printf("Erro a garantir acesso exclusivo a fila em atendimento\n");
            return NULL;
        }

        if (fila_pedidos_waiting != NULL)
        {
            ref = fila_front(fila_pedidos_waiting);
            variavel_armazenamento.type = ref->type;
            variavel_armazenamento.length = ref->length;
            variavel_armazenamento.value = ref->value;
            fila_pop(fila_pedidos_waiting);
        }
        else
        {
            printf("Erro com a fila\n");
            return NULL;
        }

        if (mutex_operation(fd_slog, n_balcao, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, n_balcao, &acesso_fila, &escrita_logs) != 0)
        {
            printf("Erro a garantir acesso exclusivo a fila em atendimento\n");
            return NULL;
        }
        //Terminado o acesso critico a fila

        int type = variavel_armazenamento.type;

        //Criar o Fifo de resposta

        switch (type)
        {
        case 0:

            if (variavel_armazenamento.value.create.account_id != ADMIN_ACCOUNT_ID)
            {
                if (open_fifo_reply(variavel_armazenamento.value.header.pid, &return_code, &fd_fifo_reply_fd) != 0)
                {
                    perror("Erro no fifo de resposta a escrever resposta:");
                    break; //Sigo para o proximo pedido
                }
            }

            if (op_create_account(variavel_armazenamento.value.header.account_id, variavel_armazenamento.value.header.password, variavel_armazenamento.value.create.account_id, variavel_armazenamento.value.create.password, variavel_armazenamento.value.create.balance, n_balcao, variavel_armazenamento.value.header.op_delay_ms, &return_code, fd_fifo_reply_fd, vec_bank_accounts) < 0)
            {
                printf("Erro na criacao da conta\n");
            }

            if (variavel_armazenamento.value.create.account_id != ADMIN_ACCOUNT_ID)
            {
                if (write_reply(fd_fifo_reply_fd, return_code, variavel_armazenamento.value.header.account_id, n_balcao, OP_CREATE_ACCOUNT) != 0)
                {
                    printf("Erro a dar reply\n");
                }
            }

            break;

        case 1:

            if (open_fifo_reply(variavel_armazenamento.value.header.pid, &return_code, &fd_fifo_reply_fd) != 0)
            {
                perror("Erro no fifo de resposta a escrever resposta:");
                //Sigo para outro pedido
            }
            else
            {
                if (op_check_balance(variavel_armazenamento.value.header.account_id, variavel_armazenamento.value.header.password, fd_fifo_reply_fd, variavel_armazenamento.value.header.op_delay_ms, return_code, n_balcao, vec_bank_accounts) != 0)
                {
                    printf("Erro na consulta\n");
                }
            }

            break;

        case 2:

            if (open_fifo_reply(variavel_armazenamento.value.header.pid, &return_code, &fd_fifo_reply_fd) == -1)
            {
                perror("Erro no fifo de resposta a escrever resposta:");
                //Sigo para outro pedido
            }
            else
            {
                if (op_make_transfer(variavel_armazenamento.value.header.account_id, variavel_armazenamento.value.header.password, variavel_armazenamento.value.transfer.account_id, variavel_armazenamento.value.transfer.amount, fd_fifo_reply_fd, variavel_armazenamento.value.header.op_delay_ms, n_balcao, &return_code, vec_bank_accounts) != 0)
                {
                    printf("Erro na transferencia\n");
                }
            }

            break;

        case 3:

            if (open_fifo_reply(variavel_armazenamento.value.header.pid, &return_code, &fd_fifo_reply_fd) != 0)
            {
                perror("Erro no fifo de resposta a escrever resposta:");
                //Sigo para outro pedido
            }
            else
            {
                if (op_server_shutdown(variavel_armazenamento.value.header.account_id, variavel_armazenamento.value.header.password, fd_fifo_reply_fd, variavel_armazenamento.value.header.op_delay_ms, &return_code, n_balcao, variavel_armazenamento.value.header.pid, vec_bank_accounts) != 0)
                {
                    printf("Erro server shutdown\n");
                }
            }

            break;

        default:
            printf("Erro a processar o atendimento. PANIC\n");
            return NULL;
        }
        if (semaphore_operation(fd_slog, n_balcao, SYNC_OP_SEM_POST, SYNC_ROLE_CONSUMER, variavel_armazenamento.value.header.pid, &sem_consumer_balcoes, PARAMETER_NOT_USED, &escrita_logs) != 0)
        {
            printf("Erro a dar post no thread\n");
            return NULL;
        }
    }

    return NULL;
}
int op_check_balance(const int account_id, char *senha, const int fifo_fd_reply, const useconds_t op_delay_ms, int return_code, const int balcao_caller, bank_account *vec_bank_accounts)
{

    if (senha == NULL)
    {
        printf("Senha a NULL no op_check_balance\n");
        return -1;
    }

    int value_ret_check_autentic = -1;
    int indice_vetor = -1;
    tlv_reply_t resposta_consulta_saldo;

    value_ret_check_autentic = check_authentication(account_id, senha, vec_bank_accounts);

    if (value_ret_check_autentic == -1)
    {
        printf("Erro de dados a NULL em op_check_balance\n");
        return -1;
    }
    else if (value_ret_check_autentic == ERR_ARRAY)
    {
        printf("Erro no acesso ao array de dados em op_check_balance\n");
        return -1;
    }
    else if (value_ret_check_autentic == UNEXISTENT_ACCOUNT)
    {
        if (return_code > RC_ID_NOT_FOUND)
        {
            return_code = RC_ID_NOT_FOUND;
        }
    }
    else if (value_ret_check_autentic == WRONG_PASSWORD)
    {

        if (return_code > RC_LOGIN_FAIL)
        {
            return_code = RC_LOGIN_FAIL;
        }
    }
    else if (value_ret_check_autentic == PASSWORD_CORRECT)
    {
        //Entrar em seccao critica de acesso ao vetor para consultar saldo

        //S.C:

        if (mutex_operation(fd_slog, balcao_caller, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, UNKNOWN_REQUEST_PID, &vec_bank_accounts[account_id].mutex_acesso_cont, &escrita_logs) != 0)
        {
            printf("Erro a fazer unlock do mutex em check balance\n");
            return -1;
        }

        if (op_delay_ms != 0)
        {

            if (pthread_mutex_lock(&escrita_logs) != 0)
            {
                perror("Erro no lock do mutex de escrita para o fdlog:");
            }

            if (logSyncDelay(fd_slog, balcao_caller, account_id, op_delay_ms) < 0)
            {
                printf("Erro a escrever para o log do server o delay em op_check_balance\n");
            }

            if (pthread_mutex_unlock(&escrita_logs) != 0)
            {
                perror("Erro no unlock do mutex de escrita para o fdlog:");
            }

            if (usleep(op_delay_ms * 1000) != 0)
            {
                perror("Erro no usleep\n");
            }
        }

        indice_vetor = account_id;

        if (indice_vetor < 0)
        {
            printf("Indice vetor invalido em op_check_balance\n");

            if (return_code > RC_OTHER)
            {
                return_code = RC_OTHER;
            }
        }
        else
        {
            resposta_consulta_saldo.value.balance.balance = vec_bank_accounts[indice_vetor].conta_info.balance;
            resposta_consulta_saldo.value.header.account_id = vec_bank_accounts[indice_vetor].conta_info.account_id;
        }

        if (mutex_operation(fd_slog, balcao_caller, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, resposta_consulta_saldo.value.header.account_id, &vec_bank_accounts[indice_vetor].mutex_acesso_cont, &escrita_logs) != 0)
        {
            printf("Erro a fazer unlock do mutex em check balance\n");
            return -1;
        }
    }

    //Fim da S.C

    /////Caso a operacao seja sucedida
    if (return_code == RET_CODE_INITIALIZATION)
    {
        return_code = RC_OK;
    }
    /////

    //Escrever resposta
    resposta_consulta_saldo.value.header.ret_code = return_code;
    resposta_consulta_saldo.type = OP_BALANCE;
    resposta_consulta_saldo.length = sizeof(resposta_consulta_saldo.value);

    if (write(fifo_fd_reply, &resposta_consulta_saldo, sizeof(resposta_consulta_saldo)) < 0)
    {
        perror("Erro na escrita para o fifo na consulta de saldo:");
        return -1;
    }

    if (pthread_mutex_lock(&escrita_logs) != 0)
    {
        perror("Erro no lock do mutex de escrita para o fdlog:");
    }

    if (logReply(fd_slog, resposta_consulta_saldo.value.header.account_id, &resposta_consulta_saldo) < 0)
    {
        printf("Erro a escrever o log em op_check_balance\n");
        return -1;
    }

    if (pthread_mutex_unlock(&escrita_logs) != 0)
    {
        perror("Erro no unlock do mutex de escrita para o fdlog:");
    }

    return 0;
}

int op_create_account(const uint32_t account_id_request, char *password_request, const uint32_t account_id_create, char *password_create, const uint32_t balance_inicial, const int balcao_caller, const useconds_t op_delay_ms, int *return_code, const int fifo_fd_reply, bank_account *vec_bank_accounts)
{
    if (password_request == NULL || password_create == NULL)
    {
        printf("Uma das Password a NULL no create_account\n");
        return -1;
    }

    int return_check_authentic = -1;

    //Para checkar a autenticacao do admin. Para o caso normal. Quando estou a criar o admin internamente nao devo verificar este valor(ele nao existe)

    if (account_id_create != ADMIN_ACCOUNT_ID)
    {
        return_check_authentic = check_authentication(account_id_request, password_request, vec_bank_accounts);

        if (return_check_authentic != PASSWORD_CORRECT)
        {
            if ((*return_code) > RC_LOGIN_FAIL)
            {
                (*return_code) = RC_LOGIN_FAIL;
            }

            return -1;
        }
    }
    //Para checkar se a conta existe
    return_check_authentic = check_authentication(account_id_create, password_create, vec_bank_accounts);

    if (return_check_authentic != UNEXISTENT_ACCOUNT)
    {
        if ((*return_code) > RC_ID_IN_USE)
        {
            (*return_code) = RC_ID_IN_USE;
        }

        return -1;
    }

    // S.C:

    if (op_delay_ms != 0)
    {

        if (pthread_mutex_lock(&escrita_logs) != 0)
        {
            perror("Erro no lock do mutex de escrita para o fdlog:");
            return -1;
        }

        if (logSyncDelay(fd_slog, balcao_caller, account_id_create, op_delay_ms) < 0)
        {
            printf("Erro a escrever para o log do server o delay em op_check_balance\n");
        }

        if (pthread_mutex_unlock(&escrita_logs) != 0)
        {
            perror("Erro no unlock do mutex de escrita para o fdlog:");
            return -1;
        }

        if (usleep(op_delay_ms * 1000) != 0)
        {
            perror("Erro no usleep\n");
        }
    }

    bank_account_t conta_nova;
    conta_nova.account_id = account_id_create;
    conta_nova.balance = balance_inicial;

    char return_salt[SALT_LEN + 1];
    char return_hash[HASH_LEN + 1];

    if (salt_generator(return_salt) != 0)
    {

        printf("Erro no salt_generator quando crio a conta\n");

        if ((*return_code) > RC_OTHER)
        {
            (*return_code) = RC_OTHER;
        }

        return -1;
    }

    if (return_salt == NULL)
    {
        printf("Impossivel gerar salt no op_creat_account\n");

        if ((*return_code) > RC_OTHER)
        {
            (*return_code) = RC_OTHER;
        }
        return -1;
    }
    else
    {
        strcpy(conta_nova.salt, return_salt);
    }

    if (sha256sum(password_create, conta_nova.salt, return_hash) == -1)
    {
        printf("sha256sum retorna erro no create_account\n");
        return -1;
    }

    if (return_hash == NULL)
    {
        printf("Impossivel gerar hash no op_creat_account\n");
        if ((*return_code) > RC_OTHER)
        {
            (*return_code) = RC_OTHER;
        }
        return -1;
    }
    else
    {
        strcpy(conta_nova.hash, return_hash);
    }

    if (mutex_operation(fd_slog, balcao_caller, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_ACCOUNT, account_id_request, &vec_bank_accounts[account_id_create].mutex_acesso_cont, &escrita_logs) != 0)
    {
        printf("Erro no mutex_operation_lock no create_account\n");
        return -1;
    }

    vec_bank_accounts[account_id_create].conta_info = conta_nova;
    vec_bank_accounts[account_id_create].initialized = ACCOUNT_ARRAY_POS_SET;

    if (mutex_operation(fd_slog, balcao_caller, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_ACCOUNT, account_id_request, &vec_bank_accounts[account_id_create].mutex_acesso_cont, &escrita_logs) != 0)
    {
        printf("Erro no mutex_operation_unlock no create_account\n");
        return -1;
    }

    if (pthread_mutex_lock(&escrita_logs) != 0)
    {
        perror("Erro no lock do mutex de escrita para o fdlog:");
    }

    if (logAccountCreation(fd_slog, balcao_caller, &conta_nova) < 0)
    {
        printf("Erro a escrever no logfile o hash\n");
    }

    if (pthread_mutex_unlock(&escrita_logs) != 0)
    {
        perror("Erro no unlock do mutex de escrita para o fdlog:");
    }

    if ((*return_code) == RC_OK || (*return_code) == RET_CODE_INITIALIZATION)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int op_make_transfer(const uint32_t account_id_origem, char *senha, const uint32_t account_id_destino, const uint32_t valor_transfer, const int fifo_fd_reply, const useconds_t op_delay_ms, const int32_t n_balcao, int *return_code, bank_account *vec_bank_accounts)
{

    if (senha == NULL || return_code == NULL || vec_bank_accounts == NULL)
    {
        {
            printf("Em op_make_transfer, a senha e passada ou return_code ou vec_bank_accounts a NULL\n");
            return -1;
        }
    }
    int return_check_authentic = -1;
    tlv_reply_t resposta_op_transfer;

    return_check_authentic = check_authentication(account_id_origem, senha, vec_bank_accounts);

    if (return_check_authentic != PASSWORD_CORRECT)
    {
        if ((*return_code) > RC_LOGIN_FAIL)
        {
            (*return_code) = RC_LOGIN_FAIL;
        }
    }
    else if (return_check_authentic == UNEXISTENT_ACCOUNT)
    {
        if ((*return_code) > RC_ID_NOT_FOUND)
        {
            (*return_code) = RC_ID_NOT_FOUND;
        }
    }

    if (vec_bank_accounts[account_id_destino].initialized == ACCOUNT_ARRAY_POS_NOT_INITIALIZED)
    {

        if ((*return_code) > RC_ID_NOT_FOUND)
        {
            (*return_code) = RC_ID_NOT_FOUND;
        }
    }

    if (op_delay_ms != 0)
    {
        if (pthread_mutex_lock(&escrita_logs) != 0)
        {
            perror("Erro no lock do mutex de escrita para o fdlog:");
        }

        if (logSyncDelay(fd_slog, n_balcao, account_id_origem, op_delay_ms) < 0)
        {
            printf("Erro a escrever para o log do server o delay em op_check_balance\n");
        }

        if (pthread_mutex_unlock(&escrita_logs) != 0)
        {
            perror("Erro no unlock do mutex de escrita para o fdlog:");
        }

        if (usleep(op_delay_ms * 1000) != 0)
        {
            perror("Erro no usleep:");
        }
    }

    int saldo_origem = -1;
    int saldo_destino = -1;

    //S.C

    if ((*return_code) == RET_CODE_INITIALIZATION)
    {

        if (mutex_operation(fd_slog, n_balcao, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, account_id_origem, &vec_bank_accounts[account_id_origem].mutex_acesso_cont, &escrita_logs) != 0)
        {
            printf("Erro a dar lock nos mutexs em make transfers\n");
            return -1;
        }

        while (mutex_operation(fd_slog, n_balcao, SYNC_OP_MUTEX_TRYLOCK, SYNC_ROLE_CONSUMER, account_id_origem, &vec_bank_accounts[account_id_destino].mutex_acesso_cont, &escrita_logs) == EBUSY)
        {

            if (pthread_mutex_lock(&escrita_logs) != 0)
            {
                perror("Erro a bloquear o log para registar o cond_wait em make transfer:");
            }

            if (logSyncMech(fd_slog, n_balcao, SYNC_OP_COND_WAIT, SYNC_ROLE_CONSUMER, account_id_origem) != 0)
            {
                printf("Erro em make transfer a fazer pthread_cond_wait\n");
            }

            if (pthread_mutex_unlock(&escrita_logs) != 0)
            {
                perror("Erro a desbloquear o log no cond_wait em make transfer:");
            }

            if (pthread_cond_wait(&condition_var_transfer, &vec_bank_accounts[account_id_origem].mutex_acesso_cont) != 0)
            {
                perror("Erro a dar o cond_wait no make_transfer:");
            }
        }

        if (mutex_operation(fd_slog, n_balcao, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, account_id_origem, &vec_bank_accounts[account_id_destino].mutex_acesso_cont, &escrita_logs) != 0)
        {
            printf("Erro a dar lock nos mutexs em make transfers\n");
            return -1;
        }

        saldo_origem = vec_bank_accounts[account_id_origem].conta_info.balance;
        saldo_destino = vec_bank_accounts[account_id_destino].conta_info.balance;

        if (saldo_origem >= valor_transfer)
        {
            // Subtrair ammount ao user;
            saldo_origem = saldo_origem - valor_transfer;
            // Adicionar ao outro user
            saldo_destino = saldo_destino + valor_transfer;

            // Atualizar os vetores
            if (saldo_destino >= MAX_BALANCE)
            {
                if ((*return_code) > RC_TOO_HIGH)
                {
                    (*return_code) = RC_TOO_HIGH;
                }
            }
            else
            {
                vec_bank_accounts[account_id_origem].conta_info.balance = saldo_origem;
                vec_bank_accounts[account_id_destino].conta_info.balance = saldo_destino;
            }
        }
        else
        {
            if ((*return_code) > RC_NO_FUNDS)
            {
                (*return_code) = RC_NO_FUNDS;
            }
        }
    }
    else if ((*return_code) == RC_ID_NOT_FOUND)
    {
        //Caso o ID de destino for errado devo retornar na mesma o valor de balance atual da conta origem
        saldo_origem = vec_bank_accounts[account_id_origem].conta_info.balance;
    }// Fim da S.C

    if (mutex_operation(fd_slog, n_balcao, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, account_id_origem, &vec_bank_accounts[account_id_origem].mutex_acesso_cont, &escrita_logs) != 0)
    {
        printf("Erro a dar lock nos mutexs em make transfers\n");
        return -1;
    }

    if (mutex_operation(fd_slog, n_balcao, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, account_id_origem, &vec_bank_accounts[account_id_destino].mutex_acesso_cont, &escrita_logs) != 0)
    {
        printf("Erro a dar lock nos mutexs em make transfers\n");
        return -1;
    }

    if (pthread_cond_signal(&condition_var_transfer) != 0)
    {
        perror("Erro a dar o cond_wait no make_transfer:");
    }

    if (pthread_mutex_lock(&escrita_logs) != 0)
    {
        perror("Erro a bloquear o log para registar o cond_wait em make transfer:");
    }

    if (logSyncMech(fd_slog, n_balcao, SYNC_OP_COND_SIGNAL, SYNC_ROLE_CONSUMER, account_id_origem) < 0)
    {
        printf("Erro em make transfer a fazer pthread_cond_signal\n");
    }

    if (pthread_mutex_unlock(&escrita_logs) != 0)
    {
        perror("Erro a desbloquear o log no cond_wait em make transfer:");
    }

    //

    if ((*return_code) == RET_CODE_INITIALIZATION)
    {
        (*return_code) = RC_OK;
    }

    resposta_op_transfer.type = OP_TRANSFER;
    resposta_op_transfer.value.transfer.balance = saldo_origem;
    resposta_op_transfer.value.header.account_id = account_id_origem;
    resposta_op_transfer.value.header.ret_code = (*return_code);
    resposta_op_transfer.length = sizeof(resposta_op_transfer.value);

    if (write(fifo_fd_reply, &resposta_op_transfer, sizeof(resposta_op_transfer)) < 0)
    {
        perror("Erro na escrita para o fifo na transferencia:");
        return -1;
    }

    if (pthread_mutex_lock(&escrita_logs) != 0)
    {
        perror("Erro no lock do mutex de escrita para o fdlog:");
    }

    if (logReply(fd_slog, n_balcao, &resposta_op_transfer) < 0)
    {
        printf("Erro a escrever a resposta em op_make_transfer\n");
    }

    if (pthread_mutex_unlock(&escrita_logs) != 0)
    {
        perror("Erro no unlock do mutex de escrita para o fdlog:");
    }

    if ((*return_code) == RC_OK || (*return_code) == RET_CODE_INITIALIZATION)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int op_server_shutdown(const int account_id, char *senha, const int fifo_fd_reply, const useconds_t op_delay_ms, int *return_code, const int balcao_caller, const int pid_request_id, bank_account *vec_bank_accounts)
{
    if (senha == NULL || return_code == NULL)
    {
        printf("Passei uma senha ou o return code a NULL em op_server_shutdown\n");
        return -1;
    }

    int check_authentication_return = -1;
    tlv_reply_t resposta_op_shutdown;

    check_authentication_return = check_authentication(account_id, senha, vec_bank_accounts);

    if (check_authentication_return == PASSWORD_CORRECT)
    {

        //So executo a tarefa se a autenticacao for a correta

        if (op_delay_ms != 0)
        {
            if (pthread_mutex_lock(&escrita_logs) != 0)
            {
                perror("Erro no lock do mutex de escrita para o fdlog:");
            }
            if (logSyncDelay(fd_slog, balcao_caller, account_id, op_delay_ms) < 0)
            {
                printf("Erro a escrever para o log do server o delay em op_server_shutdown\n");
            }

            if (pthread_mutex_unlock(&escrita_logs) != 0)
            {
                perror("Erro no unlock do mutex de escrita para o fdlog:");
            }

            if (usleep(op_delay_ms * 1000) != 0)
            {
                perror("Erro no usleep\n");
            }
        }

        if (mutex_operation(fd_slog, balcao_caller, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, pid_request_id, &vec_bank_accounts[account_id].mutex_acesso_cont, &escrita_logs) != 0)
        {
            printf("Erro a escrever no log o lock do mutex\n");
        }

        while (fila_tamanho(fila_pedidos_waiting) != 0)
        {
            if (pthread_mutex_lock(&escrita_logs) != 0)
            {
                perror("Erro no lock do mutex de escrita para o fdlog:");
            }
            if (logSyncMech(fd_slog, balcao_caller, SYNC_OP_COND_WAIT, SYNC_ROLE_CONSUMER, pid_request_id) < 0)
            {
                printf("Erro a escrever para o log do server o delay em op_server_shutdown\n");
            }

            if (pthread_mutex_unlock(&escrita_logs) != 0)
            {
                perror("Erro no unlock do mutex de escrita para o fdlog:");
            }

            if (pthread_cond_wait(&condition_var_shutdown, &vec_bank_accounts[account_id].mutex_acesso_cont) != 0)
            {
                perror("Erro no cond_wait do op_server_shutdown:");

                if ((*return_code) > RC_OTHER)
                {
                    (*return_code) = RC_OTHER;
                    break;
                }
            }
            if (logSyncMech(fd_slog, balcao_caller, SYNC_OP_COND_WAIT, SYNC_ROLE_CONSUMER, account_id) < 0)
            {
                printf("Erro a registar o cond_wait em operation shutdown\n");
            }
        }

        if (mutex_operation(fd_slog, balcao_caller, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, pid_request_id, &vec_bank_accounts[account_id].mutex_acesso_cont, &escrita_logs) < 0)
        {

            printf("Erro a escrever no log o unlock do mutex\n");
        }
    }

    if ((*return_code) == RET_CODE_INITIALIZATION)
    {
        (*return_code) = RC_OK;
    }

    int n_threads_active_on_shutdown_request = -1;

    if (sem_getvalue(&sem_consumer_balcoes, &n_threads_active_on_shutdown_request) != 0)
    {
        perror("Erro a fazer o sem_getvalue no server_shutdown:");
    }

    resposta_op_shutdown.type = OP_SHUTDOWN;
    resposta_op_shutdown.value.shutdown.active_offices = n_threads_active_on_shutdown_request;
    resposta_op_shutdown.value.header.ret_code = (*return_code);
    resposta_op_shutdown.value.header.account_id = account_id;
    resposta_op_shutdown.length = sizeof(resposta_op_shutdown.value.header);

    if (write(fifo_fd_reply, &resposta_op_shutdown, sizeof(resposta_op_shutdown)) < 0)
    {
        perror("Erro na escrita para o fifo na transferencia:");
        return -1;
    }

    if (pthread_mutex_lock(&escrita_logs) != 0)
    {
        perror("Erro no lock do mutex de escrita para o fdlog:");
    }

    if (logReply(fd_slog, balcao_caller, &resposta_op_shutdown) < 0)
    {
        printf("Erro a escrever a resposta em op_shutdown\n");
    }

    if (pthread_mutex_unlock(&escrita_logs) != 0)
    {
        perror("Erro no unlock do mutex de escrita para o fdlog:");
    }

    if ((*return_code) == RC_OK || (*return_code) == RET_CODE_INITIALIZATION)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int check_authentication(const int n_conta, char *senha, const bank_account *vec_bank_accounts)
{

    if (senha == NULL)
    {
        printf("Senha invalida passada para check_authentic\n");
        return -1;
    }
    if (vec_bank_accounts == NULL)
    {
        printf("Incapaz de aceder ao vetor de contas\n");
        return -1;
    }

    char str_sal[SALT_LEN + 1];
    char str_hash[HASH_LEN + 1];

    //Primeiro vamos encontrar a conta com esse id da conta.
    int indice_vector = n_conta;

    if (indice_vector < 0 || indice_vector >= MAX_BANK_ACCOUNTS)
    {
        printf("Erro no indice_vetor. Check authentication\n");
        return ERR_ARRAY;
    }
    else if (vec_bank_accounts[indice_vector].initialized == ACCOUNT_ARRAY_POS_NOT_INITIALIZED)
    {
        return UNEXISTENT_ACCOUNT;
    }

    //Encontrada a posicao, vamos verificar se a password e a correta

    char return_hash[HASH_LEN + 1];

    strcpy(str_sal, vec_bank_accounts[indice_vector].conta_info.salt);

    if (sha256sum(senha, str_sal, return_hash) == -1)
    {
        printf("Erro no sha256sum do check_authentication\n");
        return -1;
    }

    if (return_hash == NULL)
    {
        printf("Erro no sha256sum do check autenthic\n");
        return -1;
    }
    else
    {
        strcpy(str_hash, return_hash);
    }

    if (strcmp(str_hash, vec_bank_accounts[indice_vector].conta_info.hash) == 0)
    {
        return PASSWORD_CORRECT;
    }
    else
    {
        return WRONG_PASSWORD;
    }
}

int open_fifo_reply(pid_t pid, int *return_code, int *fifo_fd)
{
    //Utilitario para formular o fifo de resposta. Retorna um descritor onde podera ser escrito o TLV_ de resposta

    if (return_code == NULL || fifo_fd == NULL)
    {
        printf("Return code ou fifo_fd invalido em open_fifo_reply\n");
        return -1;
    }

    char fifo_path[100];

    snprintf(fifo_path, sizeof fifo_path, "%s%05u", USER_FIFO_PATH_PREFIX, pid);

    if (((*fifo_fd) = open(fifo_path, O_WRONLY)) < 0)
    {
        perror("Erro ao criar um FIFO de resposta");

        if ((*return_code) >= RC_USR_DOWN)
        {
            (*return_code) = RC_USR_DOWN;
        }

        return -1;
    }

    return 0;
}

int write_reply(const int fifo_fd_reply, int return_code, const int account_id_create, const int n_balcao, const int operation_type_to_return)
{

    tlv_reply_t resposta_op_create;

    if (return_code == RET_CODE_INITIALIZATION)
    {
        return_code = RC_OK;
    }

    resposta_op_create.type = operation_type_to_return;

    if (resposta_op_create.type == OP_CREATE_ACCOUNT)
    {
        resposta_op_create.value.header.account_id = account_id_create;
        resposta_op_create.value.header.ret_code = return_code;
        resposta_op_create.length = sizeof(resposta_op_create.value.header);
    }
    else
    {
        printf("Funcao write_reply nao concluida ainda para o pedido pretendido\n");
    }

    if (write(fifo_fd_reply, &resposta_op_create, sizeof(resposta_op_create)) < 0)
    {
        perror("Erro na escrita para o fifo na criacao de conta:");
        return -1;
    }

    if (pthread_mutex_lock(&escrita_logs) != 0)
    {
        perror("Erro no lock do mutex de escrita para o fdlog:");
    }

    if (logReply(fd_slog, n_balcao, &resposta_op_create) < 0)
    {
        printf("Erro a escrever no log do server em write_reply\n");
    }

    if (pthread_mutex_unlock(&escrita_logs) != 0)
    {
        perror("Erro no unlock do mutex de escrita para o fdlog:");
    }

    return 0;
}
