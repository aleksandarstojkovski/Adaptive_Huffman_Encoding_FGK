#include <stdio.h>
#include <stdlib.h>

int testReadBinaryFile(const char *filename);

/*
 * Programma esempio per leggere un carattere dal device urandom.
 *
 * compilazione: cc -std=c99 -o read read.c
 */
int main(int argc, char * argv[]) {
    int rc = 0;
    rc = testReadBinaryFile("../test-res/nofile.bo");
    rc = testReadBinaryFile("../test-res/32k_ff");
    rc = testReadBinaryFile("../test-res/32k_random");
    rc = testReadBinaryFile("../test-res/alice.txt");
    rc = testReadBinaryFile("../test-res/empty");
    rc = testReadBinaryFile("../test-res/ff_ff_ff");
    rc = testReadBinaryFile("../test-res/immagine.tiff");
}

// read binary file
int testReadBinaryFile(const char *filename) {
    printf("***************\n");
    printf("reading file %s\n", filename);
    printf("***************\n");

    FILE *file = NULL;
    unsigned char buffer[1024];  // array of bytes, not pointers-to-bytes
    size_t bytesRead = 0;

    file = fopen(filename, "rb");
    if(file == NULL) {
        perror("cannot open file");
        return 1;
    }

    // read up to sizeof(buffer) bytes
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        // process bytesRead worth of data in buffer
        for(int i=0;i<bytesRead;i++)
            ; //printf("%c", buffer[i]);
    }
    fclose(file);
    return 0;
}
