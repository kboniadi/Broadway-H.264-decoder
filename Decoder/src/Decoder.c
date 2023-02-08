#include "Decoder.h"

void streamInit(Stream *stream, u32 length) {
    stream->buffer = stream->pos = (u8 *)malloc(sizeof(u8) * length);
    stream->length = length;
    stream->end = stream->buffer + length;
}

void playStream(Decoder *dec, Stream *stream) {
    dec->decInput.pStream = stream->buffer;
    dec->decInput.dataLen = stream->length;
    u32 i = 0;
    do {
        u8 *start = dec->decInput.pStream;
        u32 ret = broadwayDecode(dec);
        printf("Decoded Unit #%d, Size: %d, Result: %d\n", i++, (dec->decInput.pStream - start), ret);
    } while (dec->decInput.dataLen > 0);
}



u8 *broadwayCreateStream(Decoder *dec, u32 length) {
    streamInit(&dec->broadwayStream, length);
    return dec->broadwayStream.buffer;
}

#ifdef LATENCY_OPTIMIZATION
#endif


void broadwayPlayStream(Decoder *dec, u32 length) {
    dec->broadwayStream.length = length;
    playStream(dec, &dec->broadwayStream);
}


u32 broadwayInit(Decoder *dec) {
  H264SwDecRet ret;
#ifdef DISABLE_OUTPUT_REORDERING
  u32 disableOutputReordering = 1;
#else
  u32 disableOutputReordering = 0;
#endif

  dec->streamBuffer = NULL;
  /* Initialize decoder instance. */
  ret = H264SwDecInit(&dec->decInst, disableOutputReordering);
  if (ret != H264SWDEC_OK) {
    DEBUG(("DECODER INITIALIZATION FAILED\n"));
    broadwayExit(dec);
    return -1;
  }

  dec->picDecodeNumber = dec->picDisplayNumber = 1;

  return 0;
}

u32 broadwayDecode(Decoder *dec) {
    dec->decInput.picId = dec->picDecodeNumber;

    H264SwDecRet ret = H264SwDecDecode(dec->decInst, &dec->decInput, &dec->decOutput);
    switch (ret) {
        case H264SWDEC_HDRS_RDY_BUFF_NOT_EMPTY:
            /* Stream headers were successfully decoded, thus stream information is available for query now. */
            ret = H264SwDecGetInfo(dec->decInst, &dec->decInfo);
            if (ret != H264SWDEC_OK) {
                return -1;
            }

            dec->picSize = dec->decInfo.picWidth * dec->decInfo.picHeight;
            dec->picSize = (3 * dec->picSize) / 2;

            broadwayOnHeadersDecoded();

            dec->decInput.dataLen -= dec->decOutput.pStrmCurrPos - dec->decInput.pStream;
            dec->decInput.pStream = dec->decOutput.pStrmCurrPos;
            break;

        case H264SWDEC_PIC_RDY_BUFF_NOT_EMPTY:
            /* Picture is ready and more data remains in the input buffer,
             * update input structure.
             */
            dec->decInput.dataLen -= dec->decOutput.pStrmCurrPos - dec->decInput.pStream;
            dec->decInput.pStream = dec->decOutput.pStrmCurrPos;

            /* fall through */

        case H264SWDEC_PIC_RDY:
            //if (ret == H264SWDEC_PIC_RDY) {
                dec->decInput.dataLen = 0;
            //}

            /* Increment decoding number for every decoded picture */
            dec->picDecodeNumber++;
      

            while (H264SwDecNextPicture(dec->decInst, &dec->decPicture, 0) == H264SWDEC_PIC_RDY) {
                // printf(" Decoded Picture Decode: %d, Display: %d, Type: %s\n", picDecodeNumber, picDisplayNumber, decPicture.isIdrPicture ? "IDR" : "NON-IDR");

                /* Increment display number for every displayed picture */
                dec->picDisplayNumber++;
#ifndef EMIT_IMAGE_ASAP
                broadwayOnPictureDecoded((u8*)dec->decPicture.pOutputPicture, dec->decInfo.picWidth, dec->decInfo.picHeight);
#endif
            }
            break;

      case H264SWDEC_STRM_PROCESSED:
      case H264SWDEC_STRM_ERR:
        /* Input stream was decoded but no picture is ready, thus get more data. */
        dec->decInput.dataLen = 0;
        break;
      
      default:
        break;
    }
  return ret;
}

void broadwayExit(Decoder *dec) {
    if (dec->streamBuffer) {
        free(dec->streamBuffer);
    }
}

u8 *broadwayCreateStreamBuffer(u32 size) {
    u8 *buffer = (u8 *)malloc(sizeof(u8) * size);
    if (buffer == NULL) {
        DEBUG(("UNABLE TO ALLOCATE MEMORY\n"));
    }
    return buffer;
}

u32 broadwayGetMajorVersion() {
    return H264SwDecGetAPIVersion().major;
}

u32 broadwayGetMinorVersion() {
    return H264SwDecGetAPIVersion().minor;
}

