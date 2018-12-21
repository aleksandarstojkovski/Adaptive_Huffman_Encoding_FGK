#Manual compille: gcc -lm *.c -o adaptive_huffman

CC=gcc
CFLAGS= -std=c99 -O3 -lm -I.
DEPS = *.h
OBJ = *.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

adaptive_huffman: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
