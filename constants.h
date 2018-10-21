//
// Created by massi on 15.10.2018.
//

#ifndef ALGO_CONSTANTS_H
#define ALGO_CONSTANTS_H

#include <limits.h>
#include <stdbool.h>

enum return_code {
    RC_OK = 0,
    RC_FAIL = 1
};

#define BUFFER_SIZE         1024
#define MAX_ORDER           512

#define HEADER_BITS         3
#define HEADER_DATA_BITS    5


static const short ADH_OLD_NYT_CODE = 257;
static const short ADH_NYT_CODE = 256;
static const int MAX_CODE_SIZE = 255;

static const char BIT_1 = '1';
static const char BIT_0 = '0';

#endif //ALGO_CONSTANTS_H
