#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "bin_io.h"

FILE* bin_open_read(const char *filename) {
    FILE * filePtr = fopen(filename, "rb");
    if(filePtr == NULL) {
        perror("cannot open file in [rb] mode");
    }
    return filePtr;
}

FILE* bin_open_create(const char *filename) {
    FILE * filePtr = fopen(filename, "wb");
    if(filePtr == NULL) {
        perror("cannot open file in [wb] mode");
    }
    return filePtr;
}

FILE* bin_open_update(const char *filename) {
    FILE * filePtr = fopen(filename, "rb+");
    if(filePtr == NULL) {
        perror("cannot open file in [rb+] mode");
    }
    return filePtr;
}


/*
 *  Read a binary file in chunk
 *  for each char read call a callback function
 */
int bin_read_file(const char *filename, void (*fn_process_char)(uint8_t)) {
    FILE * filePtr = bin_open_read(filename);
    if(filePtr == NULL)
        return RC_FAIL;

    uint8_t buffer[BUFFER_SIZE] = { 0 };
    size_t bytesRead = 0;
    // read up to sizeof(buffer) bytes
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), filePtr)) > 0)
    {
        for(int i=0;i<bytesRead;i++)
            fn_process_char(buffer[i]);
    }
    fclose(filePtr);
    return RC_OK;
}

/*
 * Diagnostic functions
 */
void log_trace_char_bin(uint8_t ch) {
    if(TRACE_OFF)
        return;

    for (int bitPos = CHAR_BIT-1; bitPos >= 0; --bitPos) {
        char val = bit_check(ch, bitPos);
        putchar(val);
    }
    printf("\n");
}

void log_trace_char_bin_msg(const char *msg, uint8_t ch) {
    if(TRACE_OFF)
        return;

    log_trace(msg);
    log_trace_char_bin(ch);
}

void printTime() {
    time_t rawtime;
    time (&rawtime);
    struct tm * timeinfo = localtime (&rawtime);

    char time_string[10];
    strftime(time_string, 10, "%T",timeinfo);

    printf("%s ", time_string);
}

void log_info(const char *msg, ...) {
    printTime();

    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
}

void log_trace(const char *msg, ...) {
    if(TRACE_OFF)
        return;

    va_list args;

    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
}


/*
 * return '1' if the bit at bit_pos is 1, otherwise '0'
 */
char bit_check(uint8_t ch, int bit_pos) {
    uint8_t val = (ch & (1u << bit_pos));
    return val ? BIT_1 : BIT_0;
}

void bit_set_one(uint8_t * ch, int bit_pos) {
    *ch |= (1u << bit_pos);
}

void bit_set_zero(uint8_t * ch, int bit_pos) {
    *ch  &= ~(1u << bit_pos);
}

void bit_copy(uint8_t * byte_to, uint8_t byte_from, int read_pos, int write_pos, int size) {
    for(unsigned int offset=0; offset < size; offset++) {

        unsigned int from = read_pos + offset;
        unsigned int to = write_pos + offset;

        unsigned int bit;
        bit = (byte_from >> from) & 1u;            /* Get the source bit as 0/1 symbol */
        *byte_to &= ~(1u << to);                  /* clear destination bit */
        *byte_to |= (bit << to);  /* set destination bit */
    }
}
