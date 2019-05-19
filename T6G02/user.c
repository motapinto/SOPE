#include "utilities.h"

static int filedes; //not accessible in other files
static int fd_ulogs; //not accessible in other files
static bool time_out = false; //not accessible in other files

void sigalarm_handler(int signo);
int set_alarm(int time_seg, bool set);
int doRequest(tlv_request_t *request, char *fifo_reply);
int getReply(tlv_reply_t *reply, const char *fifo_reply);
int logError(const int errnum, const tlv_request_t *req, tlv_reply_t *rep); 

// Feito por Martim
int newAccount(const uint32_t account_id, const char *password, const uint32_t op_delay_ms, const uint32_t new_account_id, const uint32_t init_balance, char* new_password)
{
    tlv_request_t  new_account_req;
    tlv_reply_t    new_account_rep;
    
    char fifo_reply[FIFO_REPLY_SIZE];
    int ret_val;

    if(password == NULL || new_password == NULL) {
        return PARAMETERS_FAIL;
    }

    //Preenche extrutura tlv_request_t
    new_account_req.type = OP_CREATE_ACCOUNT;
    new_account_req.value.header.pid = getpid();
    new_account_req.value.header.account_id = account_id;
    new_account_req.value.header.op_delay_ms = op_delay_ms;
    strcpy(new_account_req.value.header.password, password);
    
    new_account_req.value.create.account_id = new_account_id;
    new_account_req.value.create.balance =  init_balance;
    strcpy(new_account_req.value.create.password, new_password);

    //O tamanho da mensagem (pedido ou resposta) deve ser ajustado para serem trocados apenas os dados necessários
    new_account_req.length = sizeof(new_account_req.value.header) + sizeof(new_account_req.value.create);
    
    // Verifica se o pedido é realizado por admin -> não devem ser enviados pedidos invalidos
    if (account_id != ADMIN_ACCOUNT_ID) {
        logError(RC_OP_NALLOW, &new_account_req, &new_account_rep);
        ret_val = RC_OP_NALLOW;
    }

    //Escreve no fifo /tmp/secure_srv e faz log do pedido
    else if((ret_val = doRequest(&new_account_req, fifo_reply)) > RC_OK) {
        logError(ret_val,  &new_account_req, &new_account_rep);
    }

    //Preenche a struct tlv_reply_t e faz log do reply
    else if((ret_val = getReply(&new_account_rep, fifo_reply)) == RC_SRV_TIMEOUT || ret_val == RC_USR_DOWN) {
        logError(ret_val, &new_account_req, &new_account_rep);
    }

    return ret_val;
}

//Feito por Martim
int getBalance(const uint32_t account_id, const char *pass, const uint32_t op_delay_ms)
{
    tlv_request_t  balance_req;
    tlv_reply_t    balance_rep;

    char fifo_reply[FIFO_REPLY_SIZE];
    int ret_val;

    if(pass == NULL) {
        return PARAMETERS_FAIL;
    }

    //Preenche extrutura tlv_request_t
    balance_req.type = OP_BALANCE;
    balance_req.value.header.pid = getpid();
    balance_req.value.header.account_id = account_id;
    balance_req.value.header.op_delay_ms = op_delay_ms;
    strcpy(balance_req.value.header.password, pass);
    
    //O tamanho da mensagem (pedido ou resposta) deve ser ajustado para serem trocados apenas os dados necessários
    balance_req.length = sizeof(balance_req.value.header);
                         
    //Apenas clientes(id>0) podem consultar saldo
    if(account_id == ADMIN_ACCOUNT_ID) {
        logError(RC_OP_NALLOW, &balance_req, &balance_rep);
        ret_val = RC_OP_NALLOW;
    }

    //Escreve no fifo /tmp/secure_srv e faz log do pedido
    else if((ret_val = doRequest(&balance_req, fifo_reply)) > RC_OK) {
        logError(ret_val,  &balance_req, &balance_rep);
    }

     //Preenche a struct tlv_reply_t e faz log do reply
    else if((ret_val = getReply(&balance_rep, fifo_reply)) == RC_SRV_TIMEOUT || ret_val == RC_USR_DOWN) {
        logError(ret_val, &balance_req, &balance_rep);
    }
    
    return ret_val;
}

//Feito por Diogo Reis -> atualizado por Martim 
int makeTransfer(const uint32_t account_id, const char *password, const uint32_t op_delay_ms, uint32_t account_id_destination, uint32_t ammount)
{
    tlv_request_t new_transfer_req;
    tlv_reply_t new_transfer_rep;

    char fifo_reply[FIFO_REPLY_SIZE];
    int ret_val;

    if(password == NULL) {
        return PARAMETERS_FAIL;
    }

    //Preenche extrutura tlv_request_t
    new_transfer_req.type = OP_TRANSFER;
    new_transfer_req.value.header.pid = getpid();
    new_transfer_req.value.header.account_id = account_id;
    new_transfer_req.value.header.op_delay_ms = op_delay_ms;
    strcpy(new_transfer_req.value.header.password, password);

    new_transfer_req.value.transfer.account_id = account_id_destination;
    new_transfer_req.value.transfer.amount = ammount;
    new_transfer_req.length = sizeof(new_transfer_req.value.header)+sizeof(new_transfer_req.value.transfer);

    //nao se pode fazer transferências entre 2 contas com o mesmo id
    if(account_id == account_id_destination) {
        logError(RC_SAME_ID, &new_transfer_req, &new_transfer_rep);
        ret_val = RC_SAME_ID;
    }

    //Escreve no fifo /tmp/secure_srv e faz log do pedido
    if((ret_val = doRequest(&new_transfer_req, fifo_reply)) > RC_OK) {
        logError(ret_val, &new_transfer_req, &new_transfer_rep);
    }

    //Preenche a struct tlv_reply_t e faz log do reply
    else if((ret_val = getReply(&new_transfer_rep, fifo_reply)) == RC_SRV_TIMEOUT || ret_val == RC_USR_DOWN) {
        logError(ret_val, &new_transfer_req, &new_transfer_rep);
    }

    return ret_val;
}

//Feito por Martim
int userShutdown(const uint32_t account_id, const char *pass, const uint32_t op_delay_ms)
{
    tlv_request_t  usr_shutdown_req;
    tlv_reply_t    usr_shutdown_rep;
    
    char fifo_reply[FIFO_REPLY_SIZE];
    int ret_val;

    if(pass == NULL) {
        return PARAMETERS_FAIL;
    }

    //Preenche extrutura tlv_request_t
    usr_shutdown_req.type = OP_SHUTDOWN;
    usr_shutdown_req.value.header.pid = getpid();
    usr_shutdown_req.value.header.account_id = account_id;
    usr_shutdown_req.value.header.op_delay_ms = op_delay_ms;
    strcpy(usr_shutdown_req.value.header.password, pass);

    //O tamanho da mensagem (pedido ou resposta) deve ser ajustado para serem trocados apenas os dados necessários
    usr_shutdown_req.length = sizeof(usr_shutdown_req.value.header);

    //O encerramento do serviço apenas pode ser solicitado pelo administrador
    if(account_id != ADMIN_ACCOUNT_ID) {
        logError(RC_OP_NALLOW, &usr_shutdown_req, &usr_shutdown_rep);
        ret_val = RC_OP_NALLOW;
    }

    //Escreve no fifo /tmp/secure_srv e faz log do pedido
    else if((ret_val = doRequest(&usr_shutdown_req, fifo_reply)) > RC_OK) {
        logError(ret_val, &usr_shutdown_req, &usr_shutdown_rep);
    }

    //Preenche a struct tlv_reply_t e faz log do reply
    else if((ret_val = getReply(&usr_shutdown_rep, fifo_reply)) == RC_SRV_TIMEOUT || ret_val == RC_USR_DOWN) {
        logError(ret_val, &usr_shutdown_req, &usr_shutdown_rep);
    }

    //Impossibilita o envio de novos pedidos (permissoes so de leitura)---
    else if ((filedes = open(SERVER_FIFO_PATH, O_RDONLY)) == -1) {
        perror("open() em userShutdown()");
        ret_val = OPEN_ERROR;
    }

    //Para impedir o envio de novos pedidos altera permissões do FIFO secure_srv -> só de leitura
    else if(fchmod(filedes, S_IRGRP | S_IROTH | S_IRUSR ) != 0) {
        perror("fchmod() em userShutdown()");
        ret_val = FCHMOD_ERROR;
    }
    
    return ret_val;
}

//Feito por Martim
int doRequest(tlv_request_t *request, char *fifo_reply)
{
    if(fifo_reply == NULL || request == NULL) {
        fprintf(stderr, "Invalid parameters in doRequest()\n");
        return PARAMETERS_FAIL;
    }

    //O programa user deve criar o FIFO de nome /tmp/secure_XXXXX antes de enviar o pedido
    snprintf(fifo_reply, FIFO_REPLY_SIZE, "%s%05u", USER_FIFO_PATH_PREFIX, getpid()); 
    if (mkfifo(fifo_reply, READ_WRITE_ALLOW) != 0) {
        perror("mkfifo() em doRequest()");
        return MKFIFO_ERROR;
    }

    //Set alarm for 30 seg
    alarm(FIFO_TIMEOUT_SECS);   

    //Abre secure_srv para fazer novo pedido em modo não-bloqueio
    if ((filedes = open(SERVER_FIFO_PATH, O_WRONLY | O_CREAT | O_NONBLOCK)) == -1) {
        perror("open() em doRequest()");
        close(open(fifo_reply, 0));
        unlink(fifo_reply);
        return RC_SRV_DOWN;
    }

    //Escreve em secure_srv o pedido
    if (write(filedes, request, sizeof (tlv_request_t)) == -1) {
        perror("write() em doRequest()");
        close(open(fifo_reply, 0));
        unlink(fifo_reply);
        close(filedes);
        return WRITE_ERROR;
    }

    //Dá log da request
    if(logRequest(fd_ulogs, getpid(), request) < 0) {
        fprintf(stderr, "logRequest() em doRequest()\n");
        close(filedes);
        close(open(fifo_reply, 0));
        unlink(fifo_reply);
        return LOG_REQ_ERROR;
    }

    return RC_OK;
}

//Feito por Martim
int getReply(tlv_reply_t *reply, const char *fifo_reply)
{    
    //sem_t sem_rep;
    if(reply == NULL || fifo_reply == NULL) {
        return PARAMETERS_FAIL;
    }

    //Abre fifo com resposta ao pedido (pode bloquear)
    if ((filedes = open(fifo_reply, O_RDONLY)) == -1) {
        return RC_USR_DOWN;
    }

    //Lê a resposta ao pedido e preenche extrutura tlv_request_t (pode bloquear)
    if (read(filedes, reply, sizeof(tlv_reply_t)) == -1) {
        if(time_out) {
            close(filedes);
            close(open(fifo_reply, 0));
            unlink(fifo_reply);
            perror("Time out em doRequest()");
            return RC_SRV_TIMEOUT;
        }
        perror("read() em doRequest()");
        return READ_ERROR;
    }

     //Dá log da reply -> função logReply foi atualizada para a escrita dos logs ser uma secção critica
    if(logReply(fd_ulogs, getpid(), reply) < 0) {
        fprintf(stderr, "logReply() em doRequest()\n");
        close(filedes);
        close(open(fifo_reply, 0));
        unlink(fifo_reply);
        return LOG_REP_ERROR;
    }

    //"If seconds is 0, a pending alarm request, if any, is canceled."
    alarm(0);

    //Close do fifo
    if(close(filedes) != 0) {
        perror("close(filedes) em doRequest()");
        close(open(fifo_reply, 0));
        unlink(fifo_reply);
        return CLOSE_ERROR;
    }
    
    if(close(open(fifo_reply, 0)) != 0) {
        perror("close(fifo_reply) em doRequest()");
        unlink(fifo_reply);
        return CLOSE_ERROR;
    }

    //O fifo deve ser unlinked no final da chamada da função
    if(unlink(fifo_reply) != 0) {
        perror("unlink() em doRequest()");
        return UNLINK_ERROR;
    }

    return reply->value.header.ret_code;
}

//Feito por Martim
int select_operation(const uint32_t account_id, const char *pass, const uint32_t op_delay_ms, const int operation, const char *list_args) 
{
    ret_code_t ret_val;
    uint32_t new_account_id, init_balance;
    uint32_t account_id_destination, ammount;

    char arguments_list[strlen(list_args)];
    char new_pass[MAX_PASSWORD_LEN], *token;
    strcpy(arguments_list, list_args);
    char *delim = " ";

    if(pass == NULL || list_args == NULL) {
        return PARAMETERS_FAIL;
    }

    if((floor(log10(abs(account_id))) + 1) > WIDTH_ACCOUNT) {
        return PARAMETERS_FAIL;
    }

    if(strlen(pass) > MAX_PASSWORD_LEN || strlen(pass) < MIN_PASSWORD_LEN) {
        return PARAMETERS_FAIL;
    }
    
    if((floor(log10(abs(account_id))) + 1)  > WIDTH_OP || account_id < 0) {
        printf("%d\n", op_delay_ms);
        printf("%d\n", op_delay_ms % WIDTH_DELAY);
        return PARAMETERS_FAIL;
    }

    if(operation > 3 || operation < 0) {
        return PARAMETERS_FAIL;
    }
          
    switch (operation)
    {
        case OP_CREATE_ACCOUNT:
            // get the first token 
            token = strtok(arguments_list, delim);
            new_account_id = atoi(token);
            // get the second token 
            token = strtok(NULL, delim);
            init_balance = atoi(token);
            // get the third token 
            token = strtok(NULL, delim);
            strcpy(new_pass, token);

            if((floor(log10(abs(new_account_id))) + 1) > WIDTH_ACCOUNT || new_account_id > MAX_BANK_ACCOUNTS || new_account_id < 0) {
                return PARAMETERS_FAIL;
            }
            if((floor(log10(abs(init_balance))) + 1) > WIDTH_BALANCE || init_balance > MAX_BALANCE || init_balance < MIN_BALANCE) {
                return PARAMETERS_FAIL;
            }
            if(strlen(new_pass) > MAX_PASSWORD_LEN || strlen(new_pass) < MIN_PASSWORD_LEN) {
                return PARAMETERS_FAIL;
            }

            ret_val = newAccount(account_id, pass, op_delay_ms, new_account_id, init_balance, new_pass);
            break;

        case OP_BALANCE:

            ret_val = getBalance(account_id, pass, op_delay_ms);
            break;

        case OP_TRANSFER:
            // get the first token 
            token = strtok(arguments_list, delim);
            account_id_destination = atoi(token);
            // get the second token 
            token = strtok(NULL, delim);
            ammount = atoi(token);

            if((floor(log10(abs(account_id_destination))) + 1) > WIDTH_ACCOUNT || account_id_destination > MAX_BANK_ACCOUNTS || account_id_destination < 0) {
                return PARAMETERS_FAIL;
            }

            if((floor(log10(abs(ammount))) + 1) > WIDTH_BALANCE || ammount > MAX_BALANCE || ammount < 0) {
                return PARAMETERS_FAIL;
            }

            ret_val = makeTransfer(account_id, pass, op_delay_ms, account_id_destination, ammount);
            break;

        case OP_SHUTDOWN:

            ret_val = userShutdown(account_id, pass, op_delay_ms);
            break;
        
        default:
            ret_val = RC_OTHER;
    }
    return ret_val;
}

//Feito por Martim
void sigalarm_handler(int signo)
{
    time_out = true;
    //Unblocks filedes that has blocked (read call)
    //checking EINTR in read can be of any signal and isn't the best aproach
    int flags = fcntl(filedes, F_GETFL, 0);
    fcntl(filedes, F_SETFL, flags | O_NONBLOCK);
}

//Feito por Martim 
int logError(const int errnum, const tlv_request_t *req, tlv_reply_t *rep) 
{
    //Se até a função logReply houver um erro assegura a escrita nos logs user de pedidos invalidos
    //Preenche extrutura tlv_reply_t para dar log do erro 
    rep->type = req->type;
    rep->value.header.account_id = req->value.header.account_id;

    if(errnum > 0)
        rep->value.header.ret_code = errnum;
    
    //API ERROR
    else
        return 0;

    switch(req->type) {
        case OP_BALANCE:
            rep->value.balance.balance          =   0;      break;
        case OP_TRANSFER:  
            rep->value.transfer.balance         =   0;      break;
        case OP_SHUTDOWN:
            rep->value.shutdown.active_offices  =   0;      break;
        default:                                            
                                                            break;
    }    

    //Dá log do pedido com erro
    if(logRequest(fd_ulogs, getpid(), req) < 0) {
        fprintf(stderr, "logReply() em logError()\n");
        close(filedes);
        return LOG_REP_ERROR;
    }

    //Dá log da reply com erro
    if(logReply(fd_ulogs, getpid(), rep) < 0) {
        fprintf(stderr, "logReply() em logError()\n");
        close(filedes);
        return LOG_REP_ERROR;
    }

    return 0;
}

//Feito por Martim
int main(int argc, char *argv[]) 
{
    int operation;
    ret_code_t ret_val;
    char list_args[100]; 
    uint32_t account_id, op_delay_ms;
    char account_password[MAX_PASSWORD_LEN];

    //Sigaction struct for SIGALRM
    struct sigaction alarm_struct;
    alarm_struct.sa_handler = sigalarm_handler;
    sigemptyset(&alarm_struct.sa_mask);
    alarm_struct.sa_flags = 0;

    //Abre ficheiro de logs do user para escrita (fd_ulogs e global)
    fd_ulogs = open(USER_LOGFILE, O_CREAT | O_WRONLY | O_APPEND, 0777);
    if (fd_ulogs < 0) {
        perror("Log file open()");
        return OPEN_ERROR;
    }

    if(argc != 6) {
        printf("Usage: ./user <account id> <password> <operation delay> <operation> <other args>\n");
        return NUM_ARGS_ERROR;
    }

    //Preenche atributos
    account_id = atoi(argv[1]);
    strcpy(account_password, argv[2]);
    op_delay_ms = atoi(argv[3]);
    operation = atoi(argv[4]);
    strcpy(list_args, argv[5]);

    //Set handler for SIGALRM
    if (sigaction(SIGALRM, &alarm_struct, NULL) < 0) {
        perror("sigaction");
        return SIGACTION_ERROR;
    }

    //Ambos programas devem continuar a executar normalmente no caso em que uma das extremidades de comunicação seja fechada abruptamente. 
    if(ignoreSIGPIPE() != 0) {
        fprintf(stderr, "Unbale to ignore SIGPIPE");
        return IGNORE_SIGPIPE_ERROR;
    }

    ret_val = select_operation(account_id, account_password, op_delay_ms, operation, list_args);

    //Fecha ficheiro de logs do user
    if(close(fd_ulogs) != 0) {
        perror("close(fd_ulogs) em main()");
        return CLOSE_ERROR;
    }

    return ret_val;
}
