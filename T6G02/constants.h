#pragma once

#define MAX_BANK_OFFICES            99
#define MAX_BANK_ACCOUNTS           4096

#define MIN_BALANCE                 1UL
#define MAX_BALANCE                 1000000000UL

#define ADMIN_ACCOUNT_ID            0
#define MAIN_THREAD_ID              0

#define MIN_PASSWORD_LEN            8
#define MAX_PASSWORD_LEN            20
#define HASH_LEN                    64
#define SALT_LEN                    64

#define MAX_OP_DELAY_MS             99999
#define WIDTH_ID                    5
#define WIDTH_DELAY                 5
#define WIDTH_ACCOUNT               4
#define WIDTH_BALANCE               10
#define WIDTH_OP                    8
#define WIDTH_RC                    12
#define WIDTH_HASH                  5
#define WIDTH_TLV_LEN               3
#define WIDTH_STARTEND              5

#define FIFO_TIMEOUT_SECS           30
#define USER_FIFO_PATH_LEN          (sizeof(USER_FIFO_PATH_PREFIX) + WIDTH_ID + 1)

#define SERVER_LOGFILE              "slog.txt"
#define USER_LOGFILE                "ulog.txt"
#define SERVER_FIFO_PATH            "/tmp/secure_srv"
#define USER_FIFO_PATH_PREFIX       "/tmp/secure_"
#define USER_SEMAPHORE_NAME         "/sem_user_logs"

//Ruben  defines
#define UNKNOWN_REQUEST_PID          0
#define FIFO_REPLY_SIZE             18
#define END_ARRAY                   -1
#define ERR_ARRAY                   -2
#define UNEXISTENT_ACCOUNT          -3
#define WRONG_PASSWORD              -4
#define PASSWORD_CORRECT             0

#define SEM_NOT_SHARED               0
#define PARAMETER_NOT_USED          -1
#define RET_CODE_INITIALIZATION      999


//Martim defines
//file permissions
#define READ_WRITE_ALLOW            S_IRGRP | S_IRUSR | S_IROTH | S_IWGRP | S_IWUSR | S_IWOTH
#define READ_ALLOW                  S_IRGRP | S_IRUSR | S_IROTH
#define WRITE_ALLOW                 S_IWGRP | S_IWUSR | S_IWOTH

//Pipe defines
#define READ                         0
#define WRITE                        1

//exit codes:
#define FORK_ERROR                  -2
#define ALARM_ERROR                 -3
#define DUP_ERROR                   -4
#define PIPE_ERROR                  -5
#define MKFIFO_ERROR                -6
#define SIGACTION_ERROR             -7
#define SIGPROCMASK_ERROR           -8
#define SIGADDSET_ERROR             -9
#define IGNORE_SIGPIPE_ERROR        -10
#define NUM_ARGS_ERROR              -11
#define OPEN_ERROR                  -12
#define CLOSE_ERROR                 -13
#define UNLINK_ERROR                -14
#define WRITE_ERROR                 -15
#define READ_ERROR                  -16
#define LOG_REQ_ERROR               -17
#define LOG_REP_ERROR               -18
#define FCHMOD_ERROR                -19
#define PARAMETERS_FAIL             -20
#define SEMAPHORE_ERROR             -21

#define ACCOUNT_ARRAY_POS_NOT_INITIALIZED -1
#define ACCOUNT_ARRAY_POS_SET	1
