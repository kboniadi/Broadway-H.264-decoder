#ifndef DECODER_H_
#define DECODER_H_

#include "H264SwDecApi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Debug prints */
#define DEBUG(argv) printf argv

#include "opttarget.h"

#include "extraFlags.h"

const size_t STREAM_BUFFER_SIZE = 1024 * 1024;

typedef struct {
    u32 length;
    u8 *buffer;
    u8 *pos;
    u8 *end;
} Stream;

typedef struct {
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

u32 broadwayInit(Decoder *dec);
u32 broadwayDecode(Decoder *dec);
void broadwayExit(Decoder *dec);

#endif // DECODER_H_