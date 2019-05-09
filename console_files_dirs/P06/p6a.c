// PROGRAMA p6a.c ( referido na alínea a) )
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
int main(int argc, char *argv[])
{
    DIR *dirp;
    struct dirent *direntp;
    struct stat stat_buf;
    char *str;
    if (argc != 2)
    {
        fprintf( stderr, "Usage: %s dir_name/\n", argv[0]);
        exit(1);
    }
    if ((dirp = opendir( argv[1])) == NULL)
    {
        perror(argv[1]);
        exit(2);
    }
    while ((direntp = readdir( dirp)) != NULL)
    {
        char full_path[256];
        strcpy(full_path, argv[1]);
        strcat(full_path, direntp->d_name);
        if (lstat(full_path, &stat_buf) == -1) perror("Lstat err");
        if (S_ISREG(stat_buf.st_mode)) str = "regular";
        else if (S_ISDIR(stat_buf.st_mode)) str = "directory";
        else str = "other";
        //if (S_ISREG(stat_buf.st_mode)) 
            printf("%-8ld: %-25s - %-12s - %8ld bytes\n", direntp->d_ino, full_path, str, stat_buf.st_size);
        //else
            //printf("%-8ld: %-25s - %s\n", direntp->d_ino, full_path, str);
    }
    closedir(dirp);
    exit(0);
} 