#include "Decoder.h"

/*------------------------------------------------------------------------------

	Function name: NextDacket

	Purpose:
		Get the pointer to start of next packet in input stream. Uses
		global variables 'packetize' and 'nalUnitStream' to determine the
		decoder input stream mode and 'streamStop' to determine the end
		of stream. There are three possible stream modes:
			default - the whole stream at once
			packetize - a single NAL-unit with start code prefix
			nalUnitStream - a single NAL-unit without start code prefix

		pStrm stores pointer to the start of previous decoder input and is
		replaced with pointer to the start of the next decoder input.

		Returns the packet size in bytes

------------------------------------------------------------------------------*/
u32 NextPacket(Decoder *dec) {
	u32 index;
	u32 maxIndex;
	u32 zeroCount;
	u8 *stream;
	u8 byte;
	static u32 prevIndex = 0;

	/* For default stream mode all the stream is in first packet */
	if (!dec->packetize && !dec->nalUnitStream) return 0;

	index = 0;
	stream = dec->decInput.pStream + prevIndex;
	maxIndex = (u32)(dec->streamStop - stream);

	if (maxIndex == 0) return (0);

	/* leading zeros of first NAL unit */
	do {
		byte = stream[index++];
	} while (byte != 1 && index < maxIndex);

	/* invalid start code prefix */
	if (index == maxIndex || index < 3) {
		DEBUG(("INVALID BYTE STREAM\n"));
		exit(100);
	}

	/* nalUnitStream is without start code prefix */
	if (dec->nalUnitStream) {
		stream += index;
		maxIndex -= index;
		index = 0;
	}

	zeroCount = 0;

	/* Search stream for next start code prefix */
	/*lint -e(716) while(1) used consciously */
	while (1) {
		byte = stream[index++];
		if (!byte) zeroCount++;

		if ((byte == 0x01) && (zeroCount >= 2)) {
			/* Start code prefix has two zeros
			 * Third zero is assumed to be leading zero of next packet
			 * Fourth and more zeros are assumed to be trailing zeros of this
			 * packet */
			if (zeroCount > 3) {
				index -= 4;
				zeroCount -= 3;
			} else {
				index -= zeroCount + 1;
				zeroCount = 0;
			}
			break;
		} else if (byte)
			zeroCount = 0;

		if (index == maxIndex) {
			break;
		}
	}

	/* Store pointer to the beginning of the packet */
	dec->decInput.pStream = stream;
	prevIndex = index;

	/* nalUnitStream is without trailing zeros */
	if (dec->nalUnitStream) index -= zeroCount;

	return (index);
}

/*------------------------------------------------------------------------------

	Function name: CropPicture

	Purpose:
		Perform cropping for picture. Input picture pInImage with dimensions
		picWidth x picHeight is cropped with pCropParams and the resulting
		picture is stored in pOutImage.

------------------------------------------------------------------------------*/
u32 CropPicture(u8 *pOutImage, u8 *pInImage, u32 picWidth, u32 picHeight,
				CropParams *pCropParams) {
	u32 i, j;
	u32 outWidth, outHeight;
	u8 *pOut, *pIn;

	if (pOutImage == NULL || pInImage == NULL || pCropParams == NULL ||
		!picWidth || !picHeight) {
		/* just to prevent lint warning, returning non-zero will result in
		 * return without freeing the memory */
		free(pOutImage);
		return (1);
	}

	if (((pCropParams->cropLeftOffset + pCropParams->cropOutWidth) >
		 picWidth) ||
		((pCropParams->cropTopOffset + pCropParams->cropOutHeight) >
		 picHeight)) {
		/* just to prevent lint warning, returning non-zero will result in
		 * return without freeing the memory */
		free(pOutImage);
		return (1);
	}

	outWidth = pCropParams->cropOutWidth;
	outHeight = pCropParams->cropOutHeight;

	/* Calculate starting pointer for luma */
	pIn = pInImage + pCropParams->cropTopOffset * picWidth +
		  pCropParams->cropLeftOffset;
	pOut = pOutImage;

	/* Copy luma pixel values */
	for (i = outHeight; i; i--) {
		for (j = outWidth; j; j--) {
			*pOut++ = *pIn++;
		}
		pIn += picWidth - outWidth;
	}

	outWidth >>= 1;
	outHeight >>= 1;

	/* Calculate starting pointer for cb */
	pIn = pInImage + picWidth * picHeight +
		  pCropParams->cropTopOffset * picWidth / 4 +
		  pCropParams->cropLeftOffset / 2;

	/* Copy cb pixel values */
	for (i = outHeight; i; i--) {
		for (j = outWidth; j; j--) {
			*pOut++ = *pIn++;
		}
		pIn += picWidth / 2 - outWidth;
	}

	/* Calculate starting pointer for cr */
	pIn = pInImage + 5 * picWidth * picHeight / 4 +
		  pCropParams->cropTopOffset * picWidth / 4 +
		  pCropParams->cropLeftOffset / 2;

	/* Copy cr pixel values */
	for (i = outHeight; i; i--) {
		for (j = outWidth; j; j--) {
			*pOut++ = *pIn++;
		}
		pIn += picWidth / 2 - outWidth;
	}

	return (0);
}

void streamInit(Stream *stream, u32 length) {
	stream->buffer = stream->pos = (u8 *)malloc(sizeof(u8) * length);
	stream->length = length;
	stream->end = stream->buffer + length;
}

void playStream(Decoder *dec, Stream *stream) {
	dec->streamStop = stream->buffer + stream->length;
	dec->decInput.pStream = stream->buffer;
	dec->decInput.dataLen = stream->length;
	u32 i = 0;
	do {
		u8 *start = dec->decInput.pStream;
		u32 ret = broadwayDecode(dec);
		printf("Decoded Unit #%d, Size: %d, Result: %d\n", i++,
			   (dec->decInput.pStream - start), ret);
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

u32 broadwayInit(Decoder *dec, int nalUnitStream, int packetize,
				 int cropDisplay, u32 disableOutputReordering) {
	dec->nalUnitStream = nalUnitStream;
	dec->packetize = packetize;
	dec->cropDisplay = cropDisplay;
	dec->disableOutputReordering = disableOutputReordering;

	dec->streamStop = NULL;
	dec->tmpImage = NULL;

	H264SwDecRet ret;
#ifdef DISABLE_OUTPUT_REORDERING
	dec->disableOutputReordering = 1;
#else
	dec->disableOutputReordering = 0;
#endif

	dec->streamBuffer = NULL;
	/* Initialize decoder instance. */
	ret = H264SwDecInit(&dec->decInst, dec->disableOutputReordering);
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
	H264SwDecRet ret =
		H264SwDecDecode(dec->decInst, &dec->decInput, &dec->decOutput);
	switch (ret) {
	case H264SWDEC_HDRS_RDY_BUFF_NOT_EMPTY:
		/* Stream headers were successfully decoded, thus stream information is
		 * available for query now. */
		ret = H264SwDecGetInfo(dec->decInst, &dec->decInfo);
		if (ret != H264SWDEC_OK) {
			return -1;
		}

		if (dec->cropDisplay && dec->decInfo.croppingFlag) {
			/* Cropped frame size in planar YUV 4:2:0 */
			dec->picSize = dec->decInfo.cropParams.cropOutWidth *
						   dec->decInfo.cropParams.cropOutHeight;
			dec->picSize = (3 * dec->picSize) / 2;
			dec->tmpImage = malloc(dec->picSize);
			if (dec->tmpImage == NULL) return -1;
		} else {
			/* Decoder output frame size in planar YUV 4:2:0 */
			dec->picSize = dec->decInfo.picWidth * dec->decInfo.picHeight;
			dec->picSize = (3 * dec->picSize) / 2;
		}

		broadwayOnHeadersDecoded();

		/* update H264SwDecDecode() input structure, number of bytes
		 * "consumed" is computed as difference between the new stream
		 * pointer and old stream pointer
		 * */
		dec->decInput.dataLen -=
			dec->decOutput.pStrmCurrPos - dec->decInput.pStream;
		dec->decInput.pStream = dec->decOutput.pStrmCurrPos;
		break;

	case H264SWDEC_PIC_RDY_BUFF_NOT_EMPTY:
		/* Picture is ready and more data remains in input buffer
		 * -> update H264SwDecDecode() input structure, number of bytes
		 * "consumed" is computed as difference between the new stream
		 * pointer and old stream pointer */
		dec->decInput.dataLen -=
			dec->decOutput.pStrmCurrPos - dec->decInput.pStream;
		dec->decInput.pStream = dec->decOutput.pStrmCurrPos;

		/* fall through */

	case H264SWDEC_PIC_RDY:
		// if (ret == H264SWDEC_PIC_RDY) {
		dec->decInput.dataLen = NextPacket(dec);
		//}

		/* Increment decoding number for every decoded picture */
		dec->picDecodeNumber++;

		while (H264SwDecNextPicture(dec->decInst, &dec->decPicture, 0) ==
			   H264SWDEC_PIC_RDY) {
			// printf(" Decoded Picture Decode: %d, Display: %d, Type: %s\n",
			// picDecodeNumber, picDisplayNumber, decPicture.isIdrPicture ?
			// "IDR" : "NON-IDR");

			/* Increment display number for every displayed picture */
			dec->picDisplayNumber++;

			// TODO: add cropDisplay logic
			// if (cropDisplay && decInfo.croppingFlag) {
			// 	tmp = CropPicture(tmpImage, imageData, decInfo.picWidth,
			// 					  decInfo.picHeight, &decInfo.cropParams);
			// 	if (tmp) return -1;
			// 	// DrawOutput(tmpImage, decInfo.picWidth, decInfo.picHeight);
			// 	broadwayOnPictureDecoded(tmpImage, decInfo.picWidth,
			// 							 decInfo.picHeight);
			// } else {}
#ifndef EMIT_IMAGE_ASAP
			broadwayOnPictureDecoded((u8 *)dec->decPicture.pOutputPicture,
									 dec->decInfo.picWidth,
									 dec->decInfo.picHeight);
#endif
		}
		break;

	case H264SWDEC_STRM_PROCESSED:
	case H264SWDEC_STRM_ERR:
		/* Input stream was decoded but no picture is ready, thus get more data.
		 */
		dec->decInput.dataLen = NextPacket(dec);
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

u32 broadwayGetMajorVersion() { return H264SwDecGetAPIVersion().major; }

u32 broadwayGetMinorVersion() { return H264SwDecGetAPIVersion().minor; }