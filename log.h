//
// Created by Massimo De Santi on 01.11.18.
//

#ifndef ADHUFF_EXE_LOG_H
#define ADHUFF_EXE_LOG_H

#include <stdio.h>
#include <stdbool.h>
#include "constants.h"

//
// logging
//
void        log_info(const char *method, const char *format, ...);
void        log_trace(const char *method, const char *format, ...);
void        log_trace_bit_array(const byte_t *bit_array, int num_bit);
void        log_trace_char_bin(byte_t symbol);

void        set_trace_active(bool is_off);
bool        get_trace_active();


#endif //ADHUFF_EXE_LOG_H
