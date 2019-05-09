#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#define MAX_READ_LENGTH 30

int main() {

    struct termios oldterm, newterm;

    tcgetattr(STDIN_FILENO, &oldterm);
    newterm = oldterm;
    newterm.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &newterm);

    int i = 0;
    char inp, echo = '*';
    while (i < MAX_READ_LENGTH && read(STDIN_FILENO, &inp, 1) && inp != '\n') {
        write(STDOUT_FILENO, &echo, 1);
        i++;
    }

    char new_line = '\n';
    write(STDOUT_FILENO, &new_line, 1);



    tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldterm);
    return 0;
}