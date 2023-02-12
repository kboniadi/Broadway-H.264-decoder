#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <emscripten.h>
#include "Decoder.h"

#define CHUNK_SIZE 1024 * 1024

void broadwayOnPictureDecoded(u8 *buffer, u32 width, u32 height) {
    printf("am I getting called");
}

struct Nal {
    int offset;
    int end;
    int type;
};


int main(int argc, char *argv[]) {
    printf("hello, world!\n");
    printf("hello, world!\n");
    Decoder dec;
    int input_buffer_size;


    int ret = broadwayInit(&dec);
    if (ret < 0) {
        fprintf(stderr, "Failed to initialize Broadway\n");
        return -1;
    }

    FILE *input_file = fopen("mozilla_story.mp4", "rb");
    if (!input_file) {
        fprintf(stderr, "Failed to open input file\n");
        return -1;
    }

    fseek(input_file, 0, SEEK_END);
    input_buffer_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    printf("file size: %d\n", input_buffer_size);


    u8* stream = broadwayCreateStream(&dec, input_buffer_size);
    u8* buffer = (u8 *)malloc(sizeof(u8) * input_buffer_size);

    // int length = fread(buffer, sizeof(u8), input_buffer_size, input_file);
    // int pos = 0;
    // int start = 0;
    // int end = length;

    // int count = buffer[pos++] & 31;
    // printf("%d\n", count);
    // u8* sps = (u8 *)malloc(sizeof(u8) * input_buffer_size);
    // int lenSps = 0;
    // for (int i = 0; i < count; i++) {
    //     lenSps = buffer[pos + 0] << 8 | buffer[pos + 1];
    //     pos += 2;
    //     printf("%d\n", lenSps);

    //     memcpy(sps, buffer + pos, pos + lenSps);
    //     pos += lenSps;
    // }

    // count = buffer[pos++] & 31;
    // printf("%d\n", count);
    // u8* pps = (u8 *)malloc(sizeof(u8) * input_buffer_size);
    // int lenPps = 0;
    // for (int i = 0; i < count; i++) {
    //     lenPps = buffer[pos + 0] << 8 | buffer[pos + 1];
    //     pos += 2;
    //     printf("%d\n", lenPps);

    //     memcpy(pps, buffer + pos, pos + lenPps);
    //     pos += lenPps;
    // }

    // memcpy(stream, sps, lenSps);
    // broadwayPlayStream(&dec, lenSps);
    // memcpy(stream, pps, lenPps);
    // broadwayPlayStream(&dec, lenPps);

    int bytesRead = 0;
    while ((bytesRead = fread(buffer, sizeof(u8), input_buffer_size, input_file)) > 0) {
        printf("%d\n", bytesRead);
        struct Nal nals[1000];
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
                        // printf("%d %d %d\n", nals[idx].offset, nals[idx].end, nals[idx].type);
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
            // printf("%d %d %d\n", nals[idx].offset, nals[idx].end, nals[idx].type);
            idx++;
        }

        printf("-------------------------\n");
        for (int i = 0; i < 3; i++) {
            printf("arr[%d].x = %d, arr[%d].y = %d, arr[%d].z = %d\n", i, nals[i].offset, i, nals[i].end, i, nals[i].type);
        }
        int currentSlice = 0;
        int offset = 0;

        for (int i = 0; i < idx; i++) {
            int length = nals[i].end - nals[i].offset;

            stream[offset] = 0;
            offset++;
            memcpy(stream + offset, buffer + nals[i].offset, length);
            offset += length;
            for (int j = 0; j < offset; j++) {
                printf("%d\n", stream[j]);
            }
            broadwayPlayStream(&dec, offset);
            offset = 0;
        }
        // printf("%d\n", offset);
        // broadwayPlayStream(&dec, offset);
    }

    fclose(input_file);
    broadwayExit(&dec);
    printf("done\n");

    return 0;
}