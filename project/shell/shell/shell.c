#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>

#include "libunix.h"
#include "simple-boot.h"
#include "constants.h"

int fd;
int trace_p = 0;

#define MAX_TOKENS 100

typedef struct {
    int command;
    int ntoks;
    char *tokens[MAX_TOKENS];
} shell_command_t;

void send_prog(shell_command_t *cmd) {
    if (cmd->ntoks < 1) {
        output("Must supply a program to run\n");
        return;
    }

    char *prog = cmd->tokens[0];
    unsigned nbytes;
    uint8_t *code = read_file(&nbytes, prog);
    uint32_t crc = crc32(code, nbytes);

    uint8_t pis[cmd->ntoks];
    for (int i = 1; i < cmd->ntoks; i++) {
        pis[i] = atoi(cmd->tokens[i]);
    }

    put_uint32(fd, cmd->command);
    put_uint32(fd, nbytes);
    put_uint32(fd, crc);
    put_uint32(fd, cmd->ntoks - 1);
    for (int i = 1; i < cmd->ntoks; i++) {
        put_uint8(fd, pis[i]);
    }
    ck_eq32(fd, "Expected equal arguments", cmd->ntoks - 1, get_uint32(fd));
    ck_eq32(fd, "Expected equal checksums", crc, get_uint32(fd));

    put_uint32(fd, SEND_CODE);
    for (int i = 0; i < nbytes; i++) {
        put_uint8(fd, code[i]);
    }
    ck_eq32(fd, "Expected CODE_GOT", CODE_GOT, get_uint32(fd));

    uint32_t x = get_uint32(fd);
    if (x == DONE) {
        output("SUCCEESSSS");
    } else {
        output("FAILUREEE");
    }

    free(code);
}

int parse_line(char *line, shell_command_t *cmd) {
    char *delim = " ";

    char *token = strtok(line, delim);

    if (strcmp(token, "exit") == 0) {
        cmd->command = EXIT;
    } else if (strcmp(token, "run") == 0) {
        cmd->command = RUN;
    } else if (strcmp(token, "update") == 0) {
        cmd->command = UPDATE;
    } else if (strcmp(token, "list") == 0) {
        cmd->command = LIST;
    } else {
        output("Unrecognized command: %s\n", token);
        return -1;
    }

    int i = 0;
    while ((token = strtok(NULL, delim)) != NULL) {
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
    uint32_t x;

    switch (cmd->command) {
        case EXIT:
            put_uint32(fd, cmd->command);
            return -1;
        case RUN:
            send_prog(cmd);
            break;
        case UPDATE:
            break;
        case LIST:
            break;
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

    unsigned nbytes;
    uint8_t *code = read_file(&nbytes, "server.bin");
    simple_boot(fd, code, nbytes);
    free(code);

    uint32_t op;
    while ((op = get_uint32(fd)) != PI_READY) {
        output("expected initial PI_READY, got <%x>: discarding.\n", op);
        // have to remove just one byte since if not aligned, stays not aligned
        get_uint8(fd);
    }
    put_uint32(fd, UNIX_READY);
    // Drain extra PI_READY
    while ((op = get_uint32(fd)) == PI_READY) {
        output("Found extra PI_READY, got <%x>: draining.\n", op);
    }
    if (op != ACK) {
        panic("Did not get ACK from Pi\n");
    }

    int res = 1;
    while (res != -1) {
        res = command_loop();
    }

    // send_command(exits);
    output("Exiting shell\n");
}