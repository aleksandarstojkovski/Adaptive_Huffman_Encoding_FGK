#ifndef ALGO_ADHUFF_COMMON_H
#define ALGO_ADHUFF_COMMON_H

#define HEADER_BITS 3
#define HEADER_DATA_BITS 5

#pragma pack(1)
typedef struct
{
    unsigned int header: HEADER_BITS;       // bits for header
    unsigned int data: HEADER_DATA_BITS;    // rest of the bits for data
} first_byte_struct;

typedef union
{
    first_byte_struct split;
    unsigned char raw;
} first_byte_union;
#pragma pack()


#endif //ALGO_ADHUFF_COMMON_H
