#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>

#include "libunix.h"

int fd;

void parse_line(char *line) {

}

int command_loop() {
    output("Pi> ");
    char *line = NULL;
    size_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1) {
        free(line);
        return -1;
    }

    size_t len = strlen(line);
    if (line[len - 1] == '\n') {
        line[len - 1] = '\0';
        --len;
    }

    if (strcmp(line, "exit") == 0) {
        free(line);
        return -1;
    }

    output("%s\n", line);
    free(line);
    return 1;
}

int main(int argc, char *argv[]) {
    char *dev_name = find_ttyusb_last();
    if(!dev_name)
        panic("didn't find a device\n");

    int tty = open_tty(dev_name);
    if(tty < 0)
        panic("can't open tty <%s>\n", dev_name);

    double timeout_tenths = 2*5;
    unsigned baud_rate = B115200;

    fd = set_tty_to_8n1(tty, baud_rate, timeout_tenths);
    if(fd < 0)
        panic("could not set tty: <%s>\n", dev_name);

    int res = 1;
    while (res != -1) {
        res = command_loop();
    }

    // send_command(exits);
    output("Exiting shell\n");
}