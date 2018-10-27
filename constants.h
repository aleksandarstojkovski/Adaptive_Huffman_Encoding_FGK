//
// Created by massi on 15.10.2018.
//

#ifndef ALGO_CONSTANTS_H
#define ALGO_CONSTANTS_H

#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>


enum return_code {
    RC_OK = 0,
    RC_FAIL = 1
};

#define BUFFER_SIZE         1024
#define MAX_ORDER           513
#define SYMBOL_BITS         8
#define HEADER_BITS         3
#define HEADER_DATA_BITS    SYMBOL_BITS - HEADER_BITS
#define MAX_CODE_SIZE       UINT8_MAX

//static const int MAX_CODE_SIZE = 255;

static const char BIT_1 = '1';
static const char BIT_0 = '0';

static const bool TRACE_OFF = true;

#endif //ALGO_CONSTANTS_H
