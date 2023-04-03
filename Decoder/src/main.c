#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>
#include <SDL.h>
// #include "Decoder.h"
// #include "H264SwDecApi.h"
#include "Decoder.h"

// #define CHUNK_SIZE 1024 * 1024

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
int width = 1920;
int height = 816;

//! convert to baseline profile
//* ffmpeg -i h264.h264 -profile:v baseline output.h264

//! play raw YUV
//* ffplay -f rawvideo -pixel_format yuv420p -video_size 640x480 decode.yuv

void writeToFile(uint8_t* data, size_t size, const char* filename) {
    // FILE *outfile = fopen(filename, "ab");
    // if (outfile == NULL) {
    //     printf("Error opening file for writing: %s\n", filename);
    //     return;
    // }

    // size_t written = fwrite(data, 1, size, outfile);
    // if (written != size) {
    //     printf("Error writing to file: %s\n", filename);
    // } else {
    //     printf("Data written to file: %s\n", filename);
    // }

    // fclose(outfile);
    // EM_ASM(
    //     FS.syncfs(true, function(err) {
    //         if (err) {
    //             console.log("Error syncing filesystem: " + err);
    //         } else {
    //             console.log("File written successfully");
    //         }
    //     });
    // );

        // Lock the texture to get a pointer to the texture pixels
    void *pixels;
    int pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    // Copy the YUV bytes to the texture pixels pointer
    int y_size = width * height;
    int u_size = y_size / 4;
    int v_size = y_size / 4;
    unsigned char *y = data;
    unsigned char *u = data + y_size;
    unsigned char *v = data + y_size + u_size;
    unsigned char *dst = (unsigned char*)pixels;
    int dst_pitch = pitch;
    for (int i = 0; i < height; i++) {
        memcpy(dst, y, width);
        dst += dst_pitch;
        y += width;
    }
    dst = (unsigned char*)pixels + height * dst_pitch;
    for (int i = 0; i < height / 2; i++) {
        memcpy(dst, u, width / 2);
        dst += dst_pitch / 2;
        u += width / 2;
    }
    dst = (unsigned char*)pixels + height * dst_pitch * 5 / 4;
    for (int i = 0; i < height / 2; i++) {
        memcpy(dst, v, width / 2);
        dst += dst_pitch / 2;
        v += width / 2;
    }

    // Unlock the texture
    SDL_UnlockTexture(texture);
}

void broadwayOnPictureDecoded(u8 *buffer, u32 width, u32 height) {
    writeToFile(buffer, (width * height * 3) / 2, "decode.yuv");
}

void broadwayOnHeadersDecoded() {
    printf("header decoded");
}


/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

/* CHECK_MEMORY_USAGE prints and sums the memory allocated in calls to
 * H264SwDecMalloc() */
/* #define CHECK_MEMORY_USAGE */

/* _NO_OUT disables output file writing */
/* #define _NO_OUT */

/* Debug prints */
#define DEBUG(argv) printf argv

/* Global variables for stream handling */
u8 *streamStop = NULL;
u32 packetize = 0;
u32 nalUnitStream = 0;
FILE *foutput = NULL;

#define RENDER 1

// Main loop handling

enum mainLoopStatus {
  MLS_STOP = 0,
  MLS_CONTINUE = 1,
  MLS_FRAMERENDERED = 2
};

Decoder dec;
u32 i, tmp;
u32 maxNumPics = 0;
u8 *byteStrmStart;
u8 *imageData;
u8 *tmpImage = NULL;
u32 strmLen;
u32 picSize;
H264SwDecRet ret;
H264SwDecInst decInst;
H264SwDecInput decInput;
H264SwDecOutput decOutput;
H264SwDecPicture decPicture;
H264SwDecInfo decInfo;
H264SwDecApiVersion decVer;
u32 picDecodeNumber;
u32 picDisplayNumber;
u32 numErrors = 0;
u32 cropDisplay = 0;
u32 disableOutputReordering = 0;

#define NAL_HEADER_SIZE 4

int stop = 0;
void render_frame() {
    // Clear the renderer
    SDL_RenderClear(renderer);
    // Copy the texture to the renderer
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    // Render the texture to the screen
    SDL_RenderPresent(renderer);
    
    if (!stop) {
        playStream(&dec, &dec.broadwayStream);
    }

    if (dec.decInput.dataLen <= 0) {
        stop = 1;
        printf("test");
    }
}

int main(int argc, char *argv[]) {
    FILE *finput;
    

    DEBUG(("H.264 Decoder API v%d.%d\n", broadwayGetMajorVersion(), broadwayGetMinorVersion()));

    // nalUnitStream = 1;
    // packetize = 1;
    /* open input file for reading, file name given by user. If file open
     * fails -> exit */
    finput = fopen("test2.h264","rb");
    if (finput == NULL)
    {
        DEBUG(("UNABLE TO OPEN INPUT FILE\n"));
        return -1;
    }

    /* check size of the input file -> length of the stream in bytes */
    fseek(finput,0L,SEEK_END);
    strmLen = (u32)ftell(finput);
    rewind(finput);

    broadwayInit(&dec, 0, 1, 0, 0);
    byteStrmStart = broadwayCreateStream(&dec, strmLen);
    /* allocate memory for stream buffer. if unsuccessful -> exit */
    // byteStrmStart = (u8 *)malloc(sizeof(u8)*strmLen);
    // if (byteStrmStart == NULL)
    // {
    //     DEBUG(("UNABLE TO ALLOCATE MEMORY\n"));
    //     return -1;
    // }

    /* read input stream from file to buffer and close input file */
    fread(byteStrmStart, sizeof(u8), strmLen, finput);
    fclose(finput);

    /* initialize decoder. If unsuccessful -> exit */
    // ret = H264SwDecInit(&decInst, disableOutputReordering);
    // if (ret != H264SWDEC_OK)
    // {
    //     DEBUG(("DECODER INITIALIZATION FAILED\n"));
    //     free(byteStrmStart);
    //     return -1;
    // }

    DEBUG(("DECODING strmLen: %d\n", strmLen));

    /* initialize H264SwDecDecode() input structure */
    // streamStop = byteStrmStart + strmLen;
    // decInput.pStream = byteStrmStart;
    // decInput.dataLen = strmLen;
    // decInput.intraConcealmentMethod = 0;

    /* get pointer to next packet and the size of packet
     * (for packetize or nalUnitStream modes) */
    // if ( (tmp = NextPacket(&decInput.pStream)) != 0 )
    //     decInput.dataLen = tmp;

    // picDecodeNumber = picDisplayNumber = 1;

    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Create an SDL window and renderer
    window = SDL_CreateWindow("YUV Rendering", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, -1, 0);

    if (renderer == NULL) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
    }

    // Create an SDL texture
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
    // Set the render loop
    emscripten_set_main_loop(render_frame, 30, 1);

    /* main decoding loop */
    // runMainLoop();
    // EM_ASM_({ window.download() });
    return 0;
}

extern void broadwayOnFrameDecoded() {
    printf("OnFrameDecoded\n");
}

extern void broadwaySetPosition(float position) {
    printf("SetPosition %f\n", position);
}

extern float broadwayGetPosition() {
    printf("GetPosition\n");
    return 0;
}