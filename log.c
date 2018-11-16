#include <stdarg.h>

#ifdef WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif


#include <time.h>

#include "log.h"
#include "bin_io.h"

static log_level_t log_level = LOG_INFO;
//
// Diagnostic functions
//
void        print_tree(adh_node_t *node, int indent);
void        print_time(FILE* fp);
void        print_method(FILE* fp, const char *method);
void        sleep_ms(int milliseconds);

void set_log_level(log_level_t level) {
    log_level = level;
}

log_level_t get_log_level() {
    return log_level;
}

void print_time(FILE* fp) {
    time_t raw_time;
    time (&raw_time);
    struct tm * time_info = localtime(&raw_time);

    char time_string[9] = {0};
    strftime(time_string, 9, "%H:%M:%S", time_info);

    fprintf(fp, "%s  ", time_string);
}

void print_method(FILE* fp, const char *method) {
    fprintf(fp, "%-35s ", method);
}

void log_trace_char_bin(byte_t symbol) {
    if(get_log_level() < LOG_TRACE)
        return;

    bit_array_t bit_array = { 0, 0 };
    symbol_to_bits(symbol, &bit_array);
    fprintf(stdout, "%s\n", fmt_bit_array(&bit_array));
}

void log_error(const char *method, const char *format, ...) {
    if(get_log_level() < LOG_ERROR)
        return;

    // sleep some milliseconds to let info finish printing
    sleep_ms(400);

    print_time(stderr);
    print_method(stderr, method);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

/*
 * log information on screen formatting method name and adding execution time
 */
void log_info(const char *method, const char *format, ...) {
    if(get_log_level() < LOG_INFO)
        return;

    print_time(stdout);
    print_method(stdout, method);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

/*
 * same as log_info, but only if debug level is active
 */
void log_debug(const char *method, const char *format, ...) {
    if(get_log_level() < LOG_DEBUG)
        return;

    print_time(stdout);
    print_method(stdout, method);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}


/*
 * same as log_info, but only if trace level is active
 */
void log_trace(const char *method, const char *format, ...) {
    if(get_log_level() < LOG_TRACE)
        return;

    print_time(stdout);
    print_method(stdout, method);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void sleep_ms(int milliseconds) // cross-platform sleep function
{
#ifdef WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    usleep(milliseconds * 1000);
#endif
}

char * fmt_symbol(adh_symbol_t symbol) {
    static char str[MAX_SYMBOL_STR] = {0};
    switch(symbol) {
        case ADH_NYT_CODE:
            snprintf(str, sizeof(str), "NYT");
            break;
        case ADH_OLD_NYT_CODE:
            snprintf(str, sizeof(str), "ONY");
            break;
        default:
            snprintf(str, sizeof(str), "'%c'", symbol);
            break;
    }

    return str;
}

char * fmt_bit_array(const bit_array_t *bit_array) {
    static char str[MAX_BIT_STR] = {0};

    int j = 0;
    for(int i = bit_array->length-1; i>=0 && (j < sizeof(str)-2); i--) {
        str[j] = bit_array->buffer[i];
        j++;
    }

    str[j] = 0;
    return str;
}

void log_tree(adh_node_t *node) {
    if(get_log_level() < LOG_DEBUG)
        return;

    print_tree(node, 0);
    fprintf(stdout, "\n");
}

void print_tree(adh_node_t *node, int depth)
{
    if(node==NULL)
        return;

    static int nodes[MAX_ORDER];
    printf("\t");

    // unicode chars for box drawing
    // https://en.wikipedia.org/wiki/Box_Drawing
    for(int i=0;i<depth;i++) {
        if(i == depth-1)
            printf("%s\u2501\u2501\u2501\u2501\u2501\u2501 ", nodes[depth-1] ? "\u2523" : "\u2517");
        else
            printf("%s       ", nodes[i] ? "\u2503" : " ");
    }


    switch(node->symbol) {
        case ADH_NYT_CODE:
            printf("NYT (%d,%d)\n", node->weight, node->order);
            break;
        case ADH_OLD_NYT_CODE:
            printf("(%d,%d)\n", node->weight, node->order);
            break;
        default:
            printf("'%c' (%d,%d)\n", node->symbol, node->weight, node->order);
            break;
    }


    nodes[depth]=1;
    print_tree(node->left,depth+1);
    nodes[depth]=0;
    print_tree(node->right,depth+1);
}