#include "rpi.h"

int strcmp(const char *a, const char *b) {
        while (*a && *a == *b)
                ++a, ++b;
        return *a - *b;
}

