# Manual compille:
# gcc -o test_fgk ../log.c ../adhuff_decompress.c ../bin_io.c ../adhuff_compress.c  ../adhuff_common.c test.c -std=c99 -O3 -lm -Wall

CC = gcc
CFLAGS = -std=c99 -O3 -lm -Wall
OUTFILE = test_fgk
DEPS = ../*.h
OBJ = ../log.c ../adhuff_decompress.c ../bin_io.c ../adhuff_compress.c  ../adhuff_common.c test.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTFILE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
