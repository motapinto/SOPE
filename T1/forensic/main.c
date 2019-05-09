#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "file.h"
#include "signals.h"

int main(int argc, char *argv[], char *envp[])
{
   command_interpreter(argc, argv, envp);
}