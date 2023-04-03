#include <SDL.h>
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void writeToFile(uint8_t *data, size_t size, const char *filename) {
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
	unsigned char *dst = (unsigned char *)pixels;
	int dst_pitch = pitch;
	for (int i = 0; i < height; i++) {
		memcpy(dst, y, width);
		dst += dst_pitch;
		y += width;
	}
	dst = (unsigned char *)pixels + height * dst_pitch;
	for (int i = 0; i < height / 2; i++) {
		memcpy(dst, u, width / 2);
		dst += dst_pitch / 2;
		u += width / 2;
	}
	dst = (unsigned char *)pixels + height * dst_pitch * 5 / 4;
	for (int i = 0; i < height / 2; i++) {
		memcpy(dst, v, width / 2);
		dst += dst_pitch / 2;
		v += width / 2;
	}

	// Unlock the texture
	SDL_UnlockTexture(texture);
}

extern void broadwayOnPictureDecoded(u8 *buffer, u32 width, u32 height) {
	writeToFile(buffer, (width * height * 3) / 2, "decode.yuv");
}

extern void broadwayOnHeadersDecoded() { printf("header decoded"); }

/*------------------------------------------------------------------------------
	Module defines
------------------------------------------------------------------------------*/

/* CHECK_MEMORY_USAGE prints and sums the memory allocated in calls to
 * H264SwDecMalloc() */
/* #define CHECK_MEMORY_USAGE */

/* _NO_OUT disables output file writing */
/* #define _NO_OUT */

Decoder dec;
u8 *byteStrmStart;
u32 strmLen;

#define NAL_HEADER_SIZE 4

int stop = 0;
int count = 0;
void render_frame() {
	// Clear the renderer
	SDL_RenderClear(renderer);
	// Copy the texture to the renderer
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	// Render the texture to the screen
	SDL_RenderPresent(renderer);

	if (!stop) {
		// playStream(&dec, &dec.broadwayStream);
		u8 *start = dec.decInput.pStream;
		u32 ret = broadwayDecode(&dec);
		printf("Decoded Unit #%d, Size: %d, Result: %d\n", count++,
			   (dec.decInput.pStream - start), ret);
	}

	if (dec.decInput.dataLen <= 0) {
		stop = 1;
	}
}

int main(int argc, char *argv[]) {
	FILE *finput;

	DEBUG(("H.264 Decoder API v%d.%d\n", broadwayGetMajorVersion(),
		   broadwayGetMinorVersion()));

	/* open input file for reading, file name given by user. If file open
	 * fails -> exit */
	finput = fopen("test2.h264", "rb");
	if (finput == NULL) {
		DEBUG(("UNABLE TO OPEN INPUT FILE\n"));
		return -1;
	}

	/* check size of the input file -> length of the stream in bytes */
	fseek(finput, 0L, SEEK_END);
	strmLen = (u32)ftell(finput);
	rewind(finput);

	broadwayInit(&dec, 1, 0, 0, 0);
	byteStrmStart = broadwayCreateStream(&dec, strmLen);

	/* read input stream from file to buffer and close input file */
	fread(byteStrmStart, sizeof(u8), strmLen, finput);
	fclose(finput);

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

	dec.streamStop = dec.broadwayStream.buffer + dec.broadwayStream.length;
	dec.decInput.pStream = dec.broadwayStream.buffer;
	dec.decInput.dataLen = dec.broadwayStream.length;
	// Initialize SDL
	SDL_Init(SDL_INIT_VIDEO);

	// Create an SDL window and renderer
	window = SDL_CreateWindow("YUV Rendering", SDL_WINDOWPOS_UNDEFINED,
							  SDL_WINDOWPOS_UNDEFINED, width, height,
							  SDL_WINDOW_OPENGL);
	renderer = SDL_CreateRenderer(window, -1, 0);

	if (renderer == NULL) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
	}

	// Create an SDL texture
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV,
								SDL_TEXTUREACCESS_STREAMING, width, height);
	// Set the render loop
	emscripten_set_main_loop(render_frame, 30, 1);

	broadwayExit(&dec);
	/* main decoding loop */
	// runMainLoop();
	// EM_ASM_({ window.download() });
	return 0;
}

// extern void broadwayOnFrameDecoded() { printf("OnFrameDecoded\n"); }

// extern void broadwaySetPosition(float position) {
// 	printf("SetPosition %f\n", position);
// }

// extern float broadwayGetPosition() {
// 	printf("GetPosition\n");
// 	return 0;
// }