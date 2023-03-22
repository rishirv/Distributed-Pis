#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>

#include "libunix.h"

int fd;

#define MAX_TOKENS 100

enum SHELL_COMMANDS {
    EXIT
};

typedef struct {
    int command;
    int ntoks;
    char *tokens[MAX_TOKENS];
} shell_command_t;

int parse_line(char *line, shell_command_t *cmd) {
    char *delim = " ";

    char *token = strtok(line, delim);

    if (strcmp(token, "exit") == 0) {
        cmd->command = EXIT;
    } else {
        output("Unrecognized command: %s\n", token);
        return -1;
    }

    int i = 0;
    while (token != NULL) {
        token = strtok(NULL, delim);
        cmd->tokens[i++] = token;
        if (i > MAX_TOKENS) {
            output("Too many arguments\n");
            return -1;
        }
    }
    cmd->ntoks = i;

    return 1;
}

int exec_command(shell_command_t *cmd) {
    if (cmd->command == EXIT) {
        return -1;
    }
    return 1;
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

    shell_command_t command;
    if (parse_line(line, &command) < 0) {
        return 1;
    } 
    
    int res = exec_command(&command);

    free(line);
    return res;
}

int main(int argc, char *argv[]) {
    char *dev_name = find_ttyusb_last();
    if (!dev_name)
        panic("didn't find a device\n");

    int tty = open_tty(dev_name);
    if (tty < 0)
        panic("can't open tty <%s>\n", dev_name);

    double timeout_tenths = 2 * 5;
    unsigned baud_rate = B115200;

    fd = set_tty_to_8n1(tty, baud_rate, timeout_tenths);
    if (fd < 0)
        panic("could not set tty: <%s>\n", dev_name);

    int res = 1;
    while (res != -1) {
        res = command_loop();
    }

    // send_command(exits);
    output("Exiting shell\n");
}