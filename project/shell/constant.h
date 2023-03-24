enum SHELL_COMMANDS {
    EXIT,
    RUN,
    UPDATE,
    LIST
};

enum SHELL_STATUS {
    PI_READY = 0xFF00,
    UNIX_READY,
    ACK,
    SEND_CODE,
    CODE_GOT,
    DONE,
};