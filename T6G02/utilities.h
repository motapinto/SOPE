#include "constants.h"
#include "sope.h"
#include "types.h"

//Feito por Martim
int salt_generator(char * salt_str);
//Feito por Martim
int sha256sum(const char *pass, const char *salt, char* sha256sum_ret);
//Feito por Martim
int ignoreSIGPIPE(); 

//Feito por Ruben
int semaphore_operation(int fd_log, int id, sync_mech_op_t sem_operation, sync_role_t role, int sid, sem_t *semaphore_in_use, const int value_init_sem,pthread_mutex_t* escrita_log);

int mutex_operation(int fd_log, int id, sync_mech_op_t mutex_operation, sync_role_t role, int sid, pthread_mutex_t *mutex_in_use,pthread_mutex_t* escrita_log);
