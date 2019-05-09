#ifndef FILE_H
#define FILE_H
#pragma once

void print_cripto_sum(char *file_name, FILE *fp);
void recursive_call(char *file_name, int argc, char *argv[], char *envp[], FILE *fp);

int command_interpreter(int argc, char *argv[], char*envp[]);

void SIGINT_handler(int signo);
int search_envp(char *envp[], char *var);

_Bool is_directory(char *file_name);

void file_info(FILE *fp, char *file_name); 
int   file_info_size(char *fileName);
char *file_info_permissions(char *fileName);
char *return_date(char *date_string);
char *file_info_date(char *fileName);

char *file_info_file(char *fileName);
char *file_info_md5sum(char *fileName);
char *file_info_sha1sum(char *fileName);
char *file_info_sha256sum(char *fileName);
char *folder_list_files(char *fileName);

void print_event_type(char *str);

#endif