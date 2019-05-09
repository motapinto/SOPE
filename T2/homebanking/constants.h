#pragma once

#define MAX_BANK_OFFICES 99
#define MAX_BANK_ACCOUNTS 4096
#define MIN_BALANCE 1UL
#define MAX_BALANCE 1000000000UL
#define ADMIN_ACCOUNT_ID 0
#define MAIN_THREAD_ID 0
#define MIN_PASSWORD_LEN 8
#define MAX_PASSWORD_LEN 20
#define HASH_LEN 64
#define SALT_LEN 64
#define MAX_OP_DELAY_MS 99999
#define WIDTH_ID 5
#define WIDTH_DELAY 5
#define WIDTH_ACCOUNT 4
#define WIDTH_BALANCE 10
#define WIDTH_OP 8
#define WIDTH_RC 12
#define WIDTH_HASH 5
#define WIDTH_TLV_LEN 3
#define WIDTH_STARTEND 5
#define SERVER_LOGFILE "slog.txt"
#define USER_LOGFILE "ulog.txt"
#define SERVER_FIFO_PATH "/tmp/secure_srv"
#define USER_FIFO_PATH_PREFIX "/tmp/secure_"
#define USER_FIFO_PATH_LEN (sizeof(USER_FIFO_PATH_PREFIX) + WIDTH_ID + 1)
#define FIFO_TIMEOUT_SECS 30

#define UNKNOWN_REQUEST_PID 0

//OP TYPES??...
#define CREATE_ACCOUNT 0
#define CHECK_BALANCE 1
#define TRANSFER 2
#define SERVER_SHUTDOWN 3

//ARRAYS_DEFS
#define ARRAY_TIDS_INICIALIZED -1
#define ARRAY_TIDS_BUSY 0
#define ARRAY_TIDS_FREE 1

//exit codes:
#define SIGACTION_ERROR     2
#define ALARM_ERROR         3
#define FORK_ERROR          4
#define DUP_ERROR           5
#define PIPE_ERROR          6
#define MKFIFO_ERROR        7
#define SIGPROCMASK_ERROR   8

//other constants:
#define WRITE 1
#define READ  0