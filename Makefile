#Manual compille: gcc -o adaptive_huffman log.c adhuff_decompress.c bin_io.c adhuff_compress.c main.c adhuff_common.c -std=c99 -O3 -lm -I.

CC=gcc
CFLAGS= -std=c99 -O3 -lm -I.
DEPS = *.h
OBJ = *.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

adaptive_huffman: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
