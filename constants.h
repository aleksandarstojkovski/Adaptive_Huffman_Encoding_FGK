//
// Created by massi on 15.10.2018.
//

#ifndef ALGO_CONSTANTS_H
#define ALGO_CONSTANTS_H

#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>

typedef uint8_t     byte_t;

enum return_code {
    RC_OK = 0,
    RC_FAIL = 1
};

enum adh_constants {
    HEADER_BITS         = 3,
    HEADER_DATA_BITS    = 5,
    SYMBOL_BITS         = 8,
    MAX_CODE_BYTES      = 32,
    MAX_CODE_BITS       = 256,
    MAX_ORDER           = 513,
    BUFFER_SIZE         = 1024
};

static const char BIT_1 = '1';
static const char BIT_0 = '0';

#endif //ALGO_CONSTANTS_H
