#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int testReadBinaryFile(const char *filename);
void compressFile(const char *filename);
void decompressFile(const char *filename);

/*
 * compilazione: cc -std=c99 -o algo main.c
 */
int main(int argc, char* argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Not enough parameters. Usage:\n");
        printf("to compress a file: algo -c <filename_to_compress>\n");
        printf("to decompress a file: algo -d <filename_to_decompress>\n");

        printf("Start Test read binary files\n");

        int rc = 0;
        rc = testReadBinaryFile("../test-res/nofile.bo");
        rc = testReadBinaryFile("../test-res/32k_ff");
        rc = testReadBinaryFile("../test-res/32k_random");
        rc = testReadBinaryFile("../test-res/alice.txt");
        rc = testReadBinaryFile("../test-res/empty");
        rc = testReadBinaryFile("../test-res/ff_ff_ff");
        rc = testReadBinaryFile("../test-res/immagine.tiff");

        //fprintf(stderr, "Numero insufficiente di argomenti\n");
        return 1;
    }

    if (strcmp(argv[1], "-c") == 0)
        compressFile(argv[2]);
    else if (strcmp(argv[1], "-d") == 0)
        decompressFile(argv[2]);
    else {
        fprintf(stderr, "Unexpected argument\n");
        return 2;
    }

    return 0;
}

void decompressFile(const char *filename) {
    printf("START compressing: %s ...\n", filename);
}

void compressFile(const char *filename) {
    printf("START decompressing: %s ...\n", filename);
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
