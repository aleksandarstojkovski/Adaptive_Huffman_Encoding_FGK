//
// Created by Massimo De Santi on 01.11.18.
//

#ifndef ADHUFF_EXE_LOG_H
#define ADHUFF_EXE_LOG_H

#include <stdio.h>
#include <stdbool.h>
#include "constants.h"

typedef enum {
    LOG_ERROR=0,
    LOG_INFO,
    LOG_DEBUG,
    LOG_TRACE
} log_level_t;

//
// logging
//
void        log_error(const char *method, const char *format, ...);
void        log_info(const char *method, const char *format, ...);
void        log_debug(const char *method, const char *format, ...);
void        log_trace(const char *method, const char *format, ...);
void        log_trace_bit_array(const byte_t *bit_array, int num_bit);
void        log_trace_char_bin(byte_t symbol);

void        set_log_level(log_level_t level);
log_level_t get_log_level();


#endif //ADHUFF_EXE_LOG_H
