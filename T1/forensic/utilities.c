#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include "file.h"

char *octal_to_string_file_premission(int octal_value)
{
    switch (octal_value)
    {
    case 0:
        return "";
        break;
    case 1:
        return "x";
        break;
    case 2:
        return "w";
        break;
    case 3:
        return "wx";
        break;
    case 4:
        return "r";
        break;
    case 5:
        return "rx";
        break;
    case 6:
        return "rw";
        break;
    case 7:
        return "rwx";
        break;
    }
    return NULL;
}