#ifndef DECODER_H_
#define DECODER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "H264SwDecApi.h"

/* Debug prints */
#define DEBUG(argv) printf argv

#include "extraFlags.h"
#include "opttarget.h"

#define STREAM_BUFFER_SIZE = 1024 * 1024;
/* Debug prints */
#define DEBUG(argv) printf argv

typedef struct {
	u32 length;
	u8 *buffer;
	u8 *pos;
	u8 *end;
} Stream;

typedef struct {
	/*options*/
	int nalUnitStream;
	int packetize;
	int cropDisplay;
	u32 disableOutputReordering;

	u8 *streamStop;
	u8 *tmpImage;

	u8 *streamBuffer;

	H264SwDecInst decInst;
	H264SwDecInput decInput;
	H264SwDecOutput decOutput;
	H264SwDecPicture decPicture;
	H264SwDecInfo decInfo;

	u32 picDecodeNumber;
	u32 picDisplayNumber;
	u32 picSize;

	Stream stream;
	Stream broadwayStream;
} Decoder;

extern void broadwayOnHeadersDecoded();
extern void broadwayOnPictureDecoded(u8 *buffer, u32 width, u32 height);

u8 *broadwayCreateStream(Decoder *dec, u32 length);
void broadwayPlayStream(Decoder *dec, u32 length);

/*
nalUnitStream: NAL unit stream mode
packetize: packet-by-packet mode
cropDisplay: display cropped image (default decoded image)
disableOutputReordering: disable DPB output reordering
*/
u32 broadwayInit(Decoder *dec, int nalUnitStream, int packetize,
				 int cropDisplay, u32 disableOutputReordering);
u32 broadwayDecode(Decoder *dec);
void playStream(Decoder *dec, Stream *stream);

#ifdef __cplusplus
extern "C" {
#endif

void broadwayExit(Decoder *decoder);

#ifdef __cplusplus
}
#endif
// void broadwayExit(Decoder *dec);

u32 broadwayGetMajorVersion();
u32 broadwayGetMinorVersion();

#ifdef __cplusplus
}
#endif
#endif	// DECODER_H_