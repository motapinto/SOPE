#include "constants.h"
#include "sope.h"
#include "types.h"

//Feito por Martim
char * salt_generator();
//Feito por Martim
char *sha256sum(char *pass, char *salt);
//Feito por Martim
int ignoreSIGPIPE(); 