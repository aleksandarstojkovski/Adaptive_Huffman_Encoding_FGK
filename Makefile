# Manual compille:
# gcc -o adaptive_huffman log.c adhuff_decompress.c bin_io.c adhuff_compress.c main.c adhuff_common.c -std=c99 -O3 -lm

CC = gcc
CFLAGS = -std=c99 -O3 -lm -Wall
OUTFILE = adaptive_huffman
DEPS = *.h
OBJ = *.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTFILE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
