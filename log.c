#include <stdarg.h>
#include <time.h>

#include "log.h"
#include "bin_io.h"

static bool             TRACE_ACTIVE = false;

//
// Diagnostic functions
//
void        set_trace_active(bool is_active) { TRACE_ACTIVE = is_active; }
bool        get_trace_active() { return TRACE_ACTIVE; }
void        print_time();
void        print_method(const char *method);

void print_time() {
    time_t raw_time;
    time (&raw_time);
    struct tm * time_info = localtime(&raw_time);

    char time_string[10] = {0};
    strftime(time_string, 10, "%H:%M:%S", time_info);

    printf("%s\t", time_string);
}

void print_method(const char *method) {
    printf("%-30s\t", method);
}


void log_trace_char_bin(byte_t symbol) {
    if(!get_trace_active())
        return;

    byte_t bit_array[SYMBOL_BITS] = { 0 };
    symbol_to_bits(symbol, bit_array);
}

/*
 * Wrapper to printf to add execution time information
 */
void log_info(const char *method, const char *format, ...) {
    print_time();
    print_method(method);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}


/*
 * same as log_info, but only if trace is active
 */
void log_trace(const char *method, const char *format, ...) {
    if(!get_trace_active())
        return;

    print_time();
    print_method(method);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void log_trace_bit_array(const byte_t *bit_array, int num_bit) {
    if(!get_trace_active())
        return;

    for(int i = num_bit-1; i>=0; i--) {
        printf("%c", bit_array[i]);
    }
    printf("\n");

}