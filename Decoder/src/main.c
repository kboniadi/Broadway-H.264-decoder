#include <stdio.h>
#include <unistd.h>
#include <emscripten.h>
#include "Decoder.h"

#define CHUNK_SIZE 1024 * 1024

void broadwayOnPictureDecoded(u8 *buffer, u32 width, u32 height) {
    printf("here");
}

int main(int argc, char *argv[]) {
    printf("hello, world!\n");
    Decoder dec;
    int input_buffer_size;


    int ret = broadwayInit(&dec);
    if (ret < 0) {
        fprintf(stderr, "Failed to initialize Broadway\n");
        return -1;
    }

    FILE *input_file = fopen("testFile.h264", "rb");
    if (!input_file) {
        fprintf(stderr, "Failed to open input file\n");
        return -1;
    }

    fseek(input_file, 0, SEEK_END);
    input_buffer_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    printf("file size: %d\n", input_buffer_size);


    u8* stream = broadwayCreateStream(&dec, CHUNK_SIZE);
    memset(stream, 0, CHUNK_SIZE);

    int bytesRead = 0;
    while ((bytesRead = fread(stream, sizeof(unsigned char), CHUNK_SIZE, input_file)) > 0) {
        // Extract NAL units from the buffer and pass each one to the playStream function
        int i = 0;
        while (i < bytesRead) {
            int nal_size = 0;
            for (int j = i; j < bytesRead - 4; j++) {
                if (stream[j] == 0 && stream[j + 1] == 0 && stream[j + 2] == 0 && stream[j + 3] == 1) {
                    nal_size = j - i;
                    break;
                }
            }
            printf("%d\n", nal_size);
            sleep(1);
            broadwayPlayStream(&dec, nal_size);
            i += nal_size + 4;
        }
    }

    fclose(input_file);
    broadwayExit(&dec);
    printf("done\n");

    return 0;
}