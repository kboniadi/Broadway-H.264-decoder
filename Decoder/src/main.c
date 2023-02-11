#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <emscripten.h>
#include "Decoder.h"

#define CHUNK_SIZE 1024 * 1024

void broadwayOnPictureDecoded(u8 *buffer, u32 width, u32 height) {
    printf("here");
}

struct Nal {
    int offset;
    int end;
    int type;
};


int main(int argc, char *argv[]) {
    printf("hello, world!\n");
    Decoder dec;
    int input_buffer_size;


    int ret = broadwayInit(&dec);
    if (ret < 0) {
        fprintf(stderr, "Failed to initialize Broadway\n");
        return -1;
    }

    FILE *input_file = fopen("fox.mp4", "rb");
    if (!input_file) {
        fprintf(stderr, "Failed to open input file\n");
        return -1;
    }

    fseek(input_file, 0, SEEK_END);
    input_buffer_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    printf("file size: %d\n", input_buffer_size);


    u8* stream = broadwayCreateStream(&dec, CHUNK_SIZE);
    u8* buffer = (u8 *)malloc(sizeof(u8) * CHUNK_SIZE);
    memset(stream, 0, CHUNK_SIZE);
    memset(buffer, 0, CHUNK_SIZE);

    int bytesRead = 0;
    printf("here\n");
    while ((bytesRead = fread(buffer, sizeof(unsigned char), CHUNK_SIZE, input_file)) > 0) {
        // Extract NAL units from the buffer and pass each one to the playStream function
        struct Nal nals[100];
        int idx = 0;
        int i = 0;
        int startPos = 0;
        int found = 0;
        int lastFound = 0;
        int lastStart = 0;
        while (i < bytesRead) {
            if (buffer[i] == 1) {
                if (buffer[i - 1] == 0 && buffer[i - 2] == 0) {
                    startPos = i - 2;
                    if (buffer[i - 3] == 0) {
                        startPos = i - 3;
                    }

                    if (found) {
                        nals[idx].offset = lastFound;
                        nals[idx].end = startPos;
                        nals[idx].type = buffer[lastStart] & 31;
                        idx++;
                    }
                    lastFound = startPos;
                    lastStart = startPos + 3;
                    if (buffer[i - 3] == 0) {
                        lastStart = startPos + 4;
                    }
                    found = 1;
                }
            }
            i++;
        }
        if (found) {
            nals[idx].offset = lastFound;
            nals[idx].end = i;
            nals[idx].type = buffer[lastStart] & 31;
            idx++;
        }

        int currentSlice = 0;
        int offset = 0;
        u8* subarr = (u8 *)malloc(sizeof(u8) * CHUNK_SIZE);
        for (int i = 0; i < idx; i++) {
            if (nals[i].type == 1 || nals[i].type == 5) {
                if (currentSlice == idx) {
                    int length = nals[i].end - nals[i].offset + 1;
                    // u8* subarr = &nal_unit[nals[i].offset];
                    // int length = nals[i].end - nals[i].offset + 1;
                    memcpy(subarr, buffer + nals[i].offset, length);
                    stream[offset] = 0;
                    offset++;
                    memcpy(stream + offset, subarr, length);
                    offset += length;
                }
                currentSlice++;
            } else {
                int length = nals[i].end - nals[i].offset + 1;
                // u8* subarr = &nal_unit[nals[i].offset];
                // int length = nals[i].end - nals[i].offset + 1;
                memcpy(subarr, buffer + nals[i].offset, length);
                stream[offset] = 0;
                offset++;
                memcpy(stream + offset, subarr, length);
                offset += length;
                broadwayPlayStream(&dec, offset);
                offset = 0;
            }
        }
        broadwayPlayStream(&dec, offset);
    }

    fclose(input_file);
    broadwayExit(&dec);
    printf("done\n");

    return 0;
}