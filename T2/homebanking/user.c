#include "utilities.h"

int filedes;

void sigalarm_handler(int signo);
int set_alarm(int time); //time in seconds
/*
// Diogo
ret_code_t new_account(uint32_t account_id, uint32_t balance, char *password)
{
    // Verificar se o pedido é realizado por um cliente
    if (account_id <= 1)
    {
        return RC_OP_NALLOW;
    }
    // Verificar WIDTH_ACCOUNT, WIDTH_BALANCE, MAX_BANK_ACCOUNTS, MIN_BALANCE, MAX_BALANCE, MIN_PASSWORD_LEN e MAN_PASSWORD_LEN
    if (sizeof(account_id) == WIDTH_ACCOUNT && sizeof(balance) == WIDTH_BALANCE && account_id < MAX_BANK_ACCOUNTS && balance >= MIN_BALANCE && balance <= MAX_BALANCE && sizeof(password) >= MIN_PASSWORD_LEN && sizeof(password) <= MAX_PASSWORD_LEN)
    {
    }

    req_create_account_t create_account_req;

    char fifo_request_send[16] = "/tmp/secure_srv";
    char fifo_request_receive[18];
    char create_account_msg[WIDTH_BALANCE];
    op_type_t ask_create_account = OP_CREATE_ACCOUNT;

    create_account_req.account_id = account_id;
    create_account_req.balance = balance;

    set_alarm(FIFO_TIMEOUT_SECS);

    if ((filedes = mkfifo(fifo_request_send, O_WRONLY)) != 0)
    {
        perror("Password Request");
    }

    write_op_request(filedes, ask_create_account);

    if (write(filedes, &create_account_req, sizeof(create_account_req)) != 0)
    {
        perror("Create Account Request Write");
    }

    if (close(filedes) != 0)
    {
        perror("FIFO Request Send Close");
    }

    set_alarm(FIFO_TIMEOUT_SECS);

    snprintf(fifo_request_receive, sizeof fifo_request_receive, "/tmp/secure_%d", getpid());

    if ((filedes = (open(fifo_request_receive, O_RDONLY) != 0)))
    {
        perror("FIFO Open");
    }

    if (read(filedes, &create_account_req, sizeof(create_account_req)) != 0)
    {
        perror("Transfer Request Read");
    }

    snprintf(create_account_msg, WIDTH_BALANCE, "%d€", create_account_req.password);
    printf("%s\n", create_account_msg);
    return RC_OK;
}*/

//Feito por Martim
ret_code_t getBalance(uint32_t account_id, char *pass, uint32_t op_delay_ms)
{
    tlv_request_t  balance_req;
    tlv_reply_t    balance_rep;

    char fifo_request_send[16] = "/tmp/secure_srv";
    char fifo_request_receive[18];
    char balance_msg[WIDTH_BALANCE];

    balance_req.type = OP_BALANCE;
    balance_req.value.header.pid = getpid();
    balance_req.value.header.account_id = account_id;
    balance_req.value.header.op_delay_ms = op_delay_ms;
    strcpy(balance_req.value.header.password, pass);
    balance_req.length = sizeof(balance_req.value);

    //Set alarm for 30 seg
    if(set_alarm(FIFO_TIMEOUT_SECS) != 0) { //diferente da funcao alarm no que diz ao seu comportamento (flag SA_RESTART set)
        fprintf(stderr, "Erro em set_alarm(FIFO_TIMEOUT_SECS) na função check_balance\n");
        return RC_OTHER;
    }

    //Abre secure_srv para fazer pedido op_balance
    if ((filedes = open(fifo_request_send, O_WRONLY)) != 0) {
        fprintf(stderr, "makefifo fifo_request_send");
        return RC_OTHER;
    }

    //Escreve em secure_srv o pedido
    if (write(filedes, &balance_req, sizeof(balance_req)) != 0) { //escreve em secure_srv
        perror("write balance_req");
        return RC_OTHER;
    }

    //Abre fifo secure_XXXXX com resposta ao pedido
    snprintf(fifo_request_receive, sizeof fifo_request_receive, "/tmp/secure_%d", getpid()); 
    if ((filedes = (open(fifo_request_receive, O_RDONLY) != 0))) {
        perror("open fifo /tmp/secure_XXXXX");
        return RC_OTHER;
    }

    //Lê a resposta ao pedido para a extrutura  balance_req
    if (read(filedes, &balance_rep, sizeof(balance_rep)) != 0) {
        perror("read balance_rep");
        return RC_OTHER;
    }

    //Após leitura, se não tiver ficado bloqueado faz reset do alarm
    if(set_alarm(FIFO_TIMEOUT_SECS) != 0) { 
        fprintf(stderr, "set_alarm");
        return RC_OTHER;
    }

    //Imprime a mensagem, que garante que tem de tamanho WIDTH_BALANCE
    snprintf(balance_msg, WIDTH_BALANCE, "%u€", balance_rep.value.balance.balance);
    printf("%s\n", balance_msg);

    return RC_OK;
}
/*
// Diogo
ret_code_t transfer(uint32_t account_id_destination, uint32_t ammount)
{
    // ./user 2 "password" 2 1000
    // returns balance - 1000
    // user 2 gets 1000
    // account_id_destination.balance += ammount

    req_transfer_t transfer_req;

    char fifo_request_send[16] = "/tmp/secure_srv";
    char fifo_request_receive[18];
    char transfer_msg[WIDTH_BALANCE];
    op_type_t ask_transfer = OP_TRANSFER;

    transfer_req.account_id = account_id_destination;
    transfer_req.amount = ammount;

    set_alarm(FIFO_TIMEOUT_SECS);

    if ((filedes = mkfifo(fifo_request_send, O_WRONLY)) != 0)
    {
        perror("Password Request");
    }

    write_op_request(filedes, ask_transfer);

    if (write(filedes, &transfer_req, sizeof(transfer_req)) != 0)
    {
        perror("Transfer Request Write");
    }

    if (close(filedes) != 0)
    {
        perror("FIFO Request Send Close");
    }

    set_alarm(FIFO_TIMEOUT_SECS);

    snprintf(fifo_request_receive, sizeof fifo_request_receive, "/tmp/secure_%d", getpid());

    if ((filedes = (open(fifo_request_receive, O_RDONLY) != 0)))
    {
        perror("FIFO Open");
    }

    if (read(filedes, &transfer_req, sizeof(transfer_req)) != 0)
    {
        perror("Transfer Request Read");
    }

    snprintf(transfer_msg, WIDTH_BALANCE, "%d€", transfer_req.amount);
    printf("%s\n", transfer_msg);

    return RC_OK;
}*/

//Feito por Martim
ret_code_t userShutdown(uint32_t account_id, char *pass, uint32_t op_delay_ms)
{
    int fd, sem_value;
    char fifo_request_send[16] = "/tmp/secure_srv";
    char fifo_request_receive[18];

    tlv_request_t  usr_shutdown_req;
    tlv_reply_t    usr_shutdown_rep;

    usr_shutdown_req.type = OP_BALANCE;
    usr_shutdown_req.value.header.pid = getpid();
    usr_shutdown_req.value.header.account_id = account_id;
    usr_shutdown_req.value.header.op_delay_ms = op_delay_ms;
    strcpy(usr_shutdown_req.value.header.password, pass);
    usr_shutdown_req.length = sizeof(usr_shutdown_req.value);

    //Set alarm for 30 seg
    if(set_alarm(FIFO_TIMEOUT_SECS) != 0) { //diferente da funcao alarm no que diz ao seu comportamento (flag SA_RESTART set)
        fprintf(stderr, "Erro em set_alarm(FIFO_TIMEOUT_SECS) na função check_balance\n");
        return RC_OTHER;
    }

    //Abre secure_srv para fazer pedido op_shutdown
    if ((filedes = open(fifo_request_send, O_WRONLY)) != 0) {
        fprintf(stderr, "makefifo fifo_request_send");
        return RC_OTHER;
    }

    //Escreve em secure_srv o pedido
    if (write(filedes, &usr_shutdown_req, sizeof(usr_shutdown_req)) != 0) {
        perror("write usr_shutdown_req");
        return RC_OTHER;
    }

    //Abre fifo secure_XXXXX com resposta ao pedido
    snprintf(fifo_request_receive, sizeof fifo_request_receive, "/tmp/secure_%d", getpid()); 
    if ((filedes = (open(fifo_request_receive, O_RDONLY) != 0))) {
        perror("open fifo /tmp/secure_XXXXX");
        return RC_OTHER;
    }

    //Lê a resposta ao pedido para a extrutura  balance_req
    if (read(filedes, &usr_shutdown_rep, sizeof(usr_shutdown_rep)) != 0) {
        perror("read usr_shutdown_rep");
        return RC_OTHER;
    }

    //Após leitura, se não tiver ficado bloqueado faz reset do alarm
    if(set_alarm(FIFO_TIMEOUT_SECS) != 0) { 
        fprintf(stderr, "set_alarm");
        return RC_OTHER;
    }

    //O encerramento do serviço apenas pode ser solicitado pelo administrador
    if(usr_shutdown_req.value.header.account_id != 0) 
        return RC_OP_NALLOW;

    //Impossibilita o envio de novos pedidos (permissoes so de leitura)---
    if ((fd = (open("/tmp/secure_srv", O_RDONLY) != 0))) {
        perror("open fifo /tmp/secure_srv");
        return RC_OTHER;
    }

    //Para garantir o envio de novos pedidos altera permissões do FIFO secure_srv
    if(fchmod(fd, S_IRGRP | S_IROTH | S_IRUSR ) != 0) {
        perror("fchmod");
        return RC_OTHER;
    }

    //Processamento dos processos pendentes ---
    do {
        if(sem_getvalue(&sem_producter_pedidos, &sem_value) != 0) {
            perror("sem_getvalue");
        }
        else if(sem_value > 0) {
            sleep(1);
        }
    } while(sem_value > 0);

    
    
    Caso a operação seja bem-sucedida, para
    além do código de retorno (OK), será enviado como resposta o número de consumidores/threads ativos
    (a processar um pedido) no momento do envio. Caso contrário, deve ser enviado um dos seguintes
    códigos de retorno: OP_NALLOW (pedido realizado por um cliente) ou OTHER (erro não especificado).
    

    return RC_OK;
}

//Feito por Martim
void sigalarm_handler(int signo)
{
    if (close(filedes) != 0)
    {
        perror("close filedes in sigalarm_handler");
    }
}

//Feito por Martim
int set_alarm(int time) //time in seconds
{
    struct sigaction alarm_struct;

    alarm_struct.sa_handler = sigalarm_handler;
    sigemptyset(&alarm_struct.sa_mask);
    alarm_struct.sa_flags = SA_RESTART;
    
    if (sigaction(SIGALRM, &alarm_struct, NULL) < 0)
    {
        fprintf(stderr, "Unable to install SIGALARM handler!\n");
        exit(SIGACTION_ERROR);
    }

    if (alarm(time) != 0)
    {
        exit(ALARM_ERROR);
    }

    return 0;
}

//Feito por Martim
ret_code_t select_operation(uint32_t account_id, char *pass, uint32_t op_delay_ms, int operation, char *list_args) 
{
    ret_code_t ret_val;

    /*if((account_id % pow(10, WIDTH_ID)) > 0)
        return RC_OTHER;
    
    if(strlen(pass) > MAX_PASSWORD_LEN)
        return RC_OTHER;
    
    if((op_delay_ms % pow(10, WIDTH_DELAY)) > 0)
        return RC_OTHER;*/
    
    switch (operation)
    {
        case OP_CREATE_ACCOUNT:
            break;

        case OP_BALANCE:
            ret_val = getBalance(account_id, pass, op_delay_ms);
            break;

        case OP_TRANSFER:
            break;

        case OP_SHUTDOWN:
            //ret_val = userShutdown(account_id, pass, op_delay_ms);
            break;
        
        default:
            ret_val = RC_OTHER;
    }
    return ret_val;
}

//Feito por Martim
int main(int argc, char *argv[]) 
{
    int operation;
    uint32_t account_id;
    uint32_t op_delay_ms;
    ret_code_t ret_val;
    char account_password[MAX_PASSWORD_LEN];
    char list_args[100]; //100??

    if(argc != 6) {
        printf("Usage: wrong number of arguments\n");
    }

    account_id = atoi(argv[1]);
    strcpy(account_password, argv[2]); //tirar aspas?------------------------
    op_delay_ms = atoi(argv[3]);
    operation = atoi(argv[4]);
    strcpy(list_args, argv[5]);


    if(ignoreSIGPIPE() != 0) {
        fprintf(stderr, "Unbale to ignore SIGPIPE");
    }

    ret_val = select_operation(account_id, account_password, op_delay_ms, operation, list_args);
    ret_val++;

    return 0;
}