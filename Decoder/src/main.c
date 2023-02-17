#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>
// #include "Decoder.h"
#include "H264SwDecApi.h"

// #define CHUNK_SIZE 1024 * 1024

void writeToFile(uint8_t* data, size_t size, const char* filename) {
    FILE *outfile = fopen(filename, "ab");
    if (outfile == NULL) {
        printf("Error opening file for writing: %s\n", filename);
        return;
    }

    size_t written = fwrite(data, 1, size, outfile);
    if (written != size) {
        printf("Error writing to file: %s\n", filename);
    } else {
        printf("Data written to file: %s\n", filename);
    }

    fclose(outfile);
    EM_ASM(
        FS.syncfs(true, function(err) {
            if (err) {
                console.log("Error syncing filesystem: " + err);
            } else {
                console.log("File written successfully");
            }
        });
    );
}

void broadwayOnPictureDecoded(u8 *buffer, u32 width, u32 height) {
    printf("am I getting called\n");
    writeToFile(buffer, (width * height * 3) / 2, "decode.yuv");
}

// struct Nal {
//     int offset;
//     int end;
//     int type;
// };


// int main(int argc, char *argv[]) {
//     printf("hello, world!\n");
//     printf("hello, world!\n");
    
//     Decoder dec;
//     int input_buffer_size;


//     int ret = broadwayInit(&dec);
//     if (ret != H264SWDEC_OK) {
//         fprintf(stderr, "Failed to initialize Broadway\n");
//         return -1;
//     }

//     FILE *input_file = fopen("fox.264", "rb");
//     if (!input_file) {
//         fprintf(stderr, "Failed to open input file\n");
//         return -1;
//     }

//     fseek(input_file, 0, SEEK_END);
//     input_buffer_size = ftell(input_file);
//     fseek(input_file, 0, SEEK_SET);

//     printf("file size: %d\n", input_buffer_size);


//     u8* stream = broadwayCreateStream(&dec, input_buffer_size);
//     u8* buffer = (u8 *)malloc(sizeof(u8) * input_buffer_size);

//     // int length = fread(buffer, sizeof(u8), input_buffer_size, input_file);
//     // int pos = 0;
//     // int start = 0;
//     // int end = length;

//     // int count = buffer[pos++] & 31;
//     // printf("%d\n", count);
//     // u8* sps = (u8 *)malloc(sizeof(u8) * input_buffer_size);
//     // int lenSps = 0;
//     // for (int i = 0; i < count; i++) {
//     //     lenSps = buffer[pos + 0] << 8 | buffer[pos + 1];
//     //     pos += 2;
//     //     printf("%d\n", lenSps);

//     //     memcpy(sps, buffer + pos, pos + lenSps);
//     //     pos += lenSps;
//     // }

//     // count = buffer[pos++] & 31;
//     // printf("%d\n", count);
//     // u8* pps = (u8 *)malloc(sizeof(u8) * input_buffer_size);
//     // int lenPps = 0;
//     // for (int i = 0; i < count; i++) {
//     //     lenPps = buffer[pos + 0] << 8 | buffer[pos + 1];
//     //     pos += 2;
//     //     printf("%d\n", lenPps);

//     //     memcpy(pps, buffer + pos, pos + lenPps);
//     //     pos += lenPps;
//     // }

//     // memcpy(stream, sps, lenSps);
//     // broadwayPlayStream(&dec, lenSps);
//     // memcpy(stream, pps, lenPps);
//     // broadwayPlayStream(&dec, lenPps);

//     int bytesRead = 0;
//     while ((bytesRead = fread(buffer, sizeof(u8), input_buffer_size, input_file)) > 0) {
//         printf("%d\n", bytesRead);
//         struct Nal nals[1000];
//         int idx = 0;
//         int i = 0;
//         int startPos = 0;
//         int found = 0;
//         int lastFound = 0;
//         int lastStart = 0;
//         while (i < bytesRead) {
//             if (buffer[i] == 1) {
//                 if (buffer[i - 1] == 0 && buffer[i - 2] == 0) {
//                     startPos = i - 2;
//                     if (buffer[i - 3] == 0) {
//                         startPos = i - 3;
//                     }

//                     if (found) {
//                         nals[idx].offset = lastFound;
//                         nals[idx].end = startPos;
//                         nals[idx].type = buffer[lastStart] & 31;
//                         // printf("%d %d %d\n", nals[idx].offset, nals[idx].end, nals[idx].type);
//                         idx++;
//                     }
//                     lastFound = startPos;
//                     lastStart = startPos + 3;
//                     if (buffer[i - 3] == 0) {
//                         lastStart = startPos + 4;
//                     }
//                     found = 1;
//                 }
//             }
//             i++;
//         }
//         if (found) {
//             nals[idx].offset = lastFound;
//             nals[idx].end = i;
//             nals[idx].type = buffer[lastStart] & 31;
//             // printf("%d %d %d\n", nals[idx].offset, nals[idx].end, nals[idx].type);
//             idx++;
//         }

//         printf("-------------------------\n");
//         for (int i = 0; i < 3; i++) {
//             printf("arr[%d].x = %d, arr[%d].y = %d, arr[%d].z = %d\n", i, nals[i].offset, i, nals[i].end, i, nals[i].type);
//         }
//         int currentSlice = 0;
//         int offset = 0;

//         for (int i = 0; i < idx; i++) {
//             printf("%d\n", nals[i].offset);
//             int length = nals[i].end - nals[i].offset;

//             stream[offset] = 0;
//             offset++;
//             memcpy(stream + offset, buffer + nals[i].offset, length);
//             offset += length;
//             // for (int j = 0; j < offset; j++) {
//             //     printf("%d\n", stream[j]);
//             // }
//             if (nals[i].offset >= 5000) {
//                 broadwayPlayStream(&dec, offset);
//             }
//             offset = 0;
//         }
//         // printf("%d\n", offset);
//         // broadwayPlayStream(&dec, offset);
//     }

//     fclose(input_file);
//     broadwayExit(&dec);
//     printf("done\n");

//     return 0;
// }

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

/* CVS tag name for identification */
const char tagName[256] = "$Name: FIRST_ANDROID_COPYRIGHT $";

u32 NextPacket(u8 **pStrm);
u32 CropPicture(u8 *pOutImage, u8 *pInImage,
    u32 picWidth, u32 picHeight, CropParams *pCropParams);

/* Global variables for stream handling */
u8 *streamStop = NULL;
u32 packetize = 0;
u32 nalUnitStream = 0;
FILE *foutput = NULL;

#define RENDER 1
void DrawOutput(u8 *data, u32 picWidth, u32 picHeight);

// Main loop handling

enum mainLoopStatus {
  MLS_STOP = 0,
  MLS_CONTINUE = 1,
  MLS_FRAMERENDERED = 2
};

// Runs the main loop. This is replaced in JavaScript with an asynchronous loop
// that calls mainLoopIteration
void runMainLoop();
int mainLoopIteration();

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

typedef struct {
  uint8_t *data;
  size_t size;
} NALUnit;

NALUnit *splitNals(const uint8_t *data, size_t size, size_t *numNals) {
  NALUnit *nals = NULL;
  size_t maxNals = 0;
  size_t numParsedBytes = 0;
  uint8_t nalHeader = 0;
  size_t nalSize = 0;

  while (numParsedBytes < size) {
    // Find the start of the next NAL unit
    while (numParsedBytes < size && (data[numParsedBytes] != 0 ||
                                     data[numParsedBytes + 1] != 0 ||
                                     data[numParsedBytes + 2] != 1)) {
      numParsedBytes++;
    }

    if (numParsedBytes >= size) {
      break;
    }

    if (*numNals >= maxNals) {
      maxNals += 10;
      nals = realloc(nals, sizeof(NALUnit) * maxNals);
    }

    // Read the NAL header
    nalHeader = data[numParsedBytes + 3];

    // Find the end of the NAL unit
    nalSize = numParsedBytes + NAL_HEADER_SIZE;
    while (nalSize < size &&
           !(data[nalSize - 3] == 0 && data[nalSize - 2] == 0 &&
             data[nalSize - 1] == 1)) {
      nalSize++;
    }

    // Create a new NAL unit
    size_t nalDataSize = nalSize - numParsedBytes - NAL_HEADER_SIZE;
    uint8_t *nalData = malloc(nalDataSize);
    memcpy(nalData, &data[numParsedBytes + NAL_HEADER_SIZE], nalDataSize);

    nals[*numNals].data = nalData;
    nals[*numNals].size = nalDataSize;
    (*numNals)++;

    numParsedBytes = nalSize;
  }

  return nals;
}

void runMainLoop() {
    int status;
    do {
        mainLoopIteration();
    /* keep decoding until all data from input stream buffer consumed */
    } while (decInput.dataLen > 0);

    // while ((status = mainLoopIteration()) != MLS_STOP);
}

int main(int argc, char *argv[]) {
    FILE *finput;
    
    char outFileName[256] = "";

    /* Print API version number */
    decVer = H264SwDecGetAPIVersion();

    DEBUG(("H.264 Decoder API v%d.%d\n", decVer.major, decVer.minor));

    /* Print tag name if '-T' argument present */
    // if ( argc > 1 && strcmp(argv[1], "-T") == 0 )
    // {
    //     DEBUG(("%s\n", tagName));
    //     return 0;
    // }

    /* Check that enough command line arguments given, if not -> print usage
     * information out */
//     if (argc < 2)
//     {
//         DEBUG((
//             "Usage: %s [-Nn] [-Ooutfile] [-P] [-U] [-C] [-R] [-T] file.h264\n",
//             argv[0]));
//         DEBUG(("\t-Nn forces decoding to stop after n pictures\n"));
// #if defined(_NO_OUT)
//         DEBUG(("\t-Ooutfile output writing disabled at compile time\n"));
// #else
//         DEBUG(("\t-Ooutfile write output to \"outfile\" (default out_wxxxhyyy.yuv)\n"));
//         DEBUG(("\t-Onone does not write output\n"));
// #endif
//         DEBUG(("\t-P packet-by-packet mode\n"));
//         DEBUG(("\t-U NAL unit stream mode\n"));
//         DEBUG(("\t-C display cropped image (default decoded image)\n"));
//         DEBUG(("\t-R disable DPB output reordering\n"));
//         DEBUG(("\t-T to print tag name and exit\n"));
//         return 0;
//     }

    /* read command line arguments */
    for (i = 1; i < (u32)(argc-1); i++)
    {
        if ( strncmp(argv[i], "-N", 2) == 0 )
        {
            maxNumPics = (u32)atoi(argv[i]+2);
        }
        else if ( strncmp(argv[i], "-O", 2) == 0 )
        {
            strcpy(outFileName, argv[i]+2);
        }
        else if ( strcmp(argv[i], "-P") == 0 )
        {
            packetize = 1;
        }
        else if ( strcmp(argv[i], "-U") == 0 )
        {
            nalUnitStream = 1;
        }
        else if ( strcmp(argv[i], "-C") == 0 )
        {
            cropDisplay = 1;
        }
        else if ( strcmp(argv[i], "-R") == 0 )
        {
            disableOutputReordering = 1;
        }
    }
    nalUnitStream = 1;
    // packetize = 1;
    /* open input file for reading, file name given by user. If file open
     * fails -> exit */
    finput = fopen("output.h264","rb");
    if (finput == NULL)
    {
        DEBUG(("UNABLE TO OPEN INPUT FILE\n"));
        return -1;
    }

    /* check size of the input file -> length of the stream in bytes */
    fseek(finput,0L,SEEK_END);
    strmLen = (u32)ftell(finput);
    rewind(finput);

    /* allocate memory for stream buffer. if unsuccessful -> exit */
    byteStrmStart = (u8 *)malloc(sizeof(u8)*strmLen);
    if (byteStrmStart == NULL)
    {
        DEBUG(("UNABLE TO ALLOCATE MEMORY\n"));
        return -1;
    }

    /* read input stream from file to buffer and close input file */
    fread(byteStrmStart, sizeof(u8), strmLen, finput);
    fclose(finput);

    /* initialize decoder. If unsuccessful -> exit */
    ret = H264SwDecInit(&decInst, disableOutputReordering);
    if (ret != H264SWDEC_OK)
    {
        DEBUG(("DECODER INITIALIZATION FAILED\n"));
        free(byteStrmStart);
        return -1;
    }

    DEBUG(("DECODING strmLen: %d\n", strmLen));

    /* initialize H264SwDecDecode() input structure */
    streamStop = byteStrmStart + strmLen;
    decInput.pStream = byteStrmStart;
    decInput.dataLen = strmLen;
    decInput.intraConcealmentMethod = 0;

    /* get pointer to next packet and the size of packet
     * (for packetize or nalUnitStream modes) */
    if ( (tmp = NextPacket(&decInput.pStream)) != 0 )
        decInput.dataLen = tmp;

    picDecodeNumber = picDisplayNumber = 1;

    /* main decoding loop */
    runMainLoop();
    EM_ASM_({ window.download() });
    return 0;
}

int mainLoopIteration() {
    /* Picture ID is the picture number in decoding order */
    decInput.picId = picDecodeNumber;
    
    /* call API function to perform decoding */
    ret = H264SwDecDecode(decInst, &decInput, &decOutput);
    // printf("%dasdf\n", ret);
    switch(ret)
    {

        case H264SWDEC_HDRS_RDY_BUFF_NOT_EMPTY:
            /* Stream headers were successfully decoded
             * -> stream information is available for query now */

            ret = H264SwDecGetInfo(decInst, &decInfo);
            if (ret != H264SWDEC_OK)
                return -1;

            DEBUG(("Profile %d\n", decInfo.profile));

            DEBUG(("Width %d Height %d\n",
                decInfo.picWidth, decInfo.picHeight));

            if (cropDisplay && decInfo.croppingFlag)
            {
                DEBUG(("Cropping params: (%d, %d) %dx%d\n",
                    decInfo.cropParams.cropLeftOffset,
                    decInfo.cropParams.cropTopOffset,
                    decInfo.cropParams.cropOutWidth,
                    decInfo.cropParams.cropOutHeight));

                /* Cropped frame size in planar YUV 4:2:0 */
                picSize = decInfo.cropParams.cropOutWidth *
                          decInfo.cropParams.cropOutHeight;
                picSize = (3 * picSize)/2;
                tmpImage = malloc(picSize);
                if (tmpImage == NULL)
                    return -1;
            }
            else
            {
                /* Decoder output frame size in planar YUV 4:2:0 */
                picSize = decInfo.picWidth * decInfo.picHeight;
                picSize = (3 * picSize)/2;
            }

            DEBUG(("videoRange %d, matrixCoefficients %d\n",
                decInfo.videoRange, decInfo.matrixCoefficients));

            /* update H264SwDecDecode() input structure, number of bytes
             * "consumed" is computed as difference between the new stream
             * pointer and old stream pointer */
            decInput.dataLen -=
                (u32)(decOutput.pStrmCurrPos - decInput.pStream);
            decInput.pStream = decOutput.pStrmCurrPos;
            break;

        case H264SWDEC_PIC_RDY_BUFF_NOT_EMPTY:
            /* Picture is ready and more data remains in input buffer
             * -> update H264SwDecDecode() input structure, number of bytes
             * "consumed" is computed as difference between the new stream
             * pointer and old stream pointer */
            decInput.dataLen -=
                (u32)(decOutput.pStrmCurrPos - decInput.pStream);
            decInput.pStream = decOutput.pStrmCurrPos;
            /* fall through */

        case H264SWDEC_PIC_RDY:

            /*lint -esym(644,tmpImage,picSize) variable initialized at
             * H264SWDEC_HDRS_RDY_BUFF_NOT_EMPTY case */

            if (ret == H264SWDEC_PIC_RDY)
                decInput.dataLen = NextPacket(&decInput.pStream);

            /* If enough pictures decoded -> force decoding to end
             * by setting that no more stream is available */
            if (maxNumPics && picDecodeNumber == maxNumPics)
                decInput.dataLen = 0;

            /* Increment decoding number for every decoded picture */
            picDecodeNumber++;

            /* use function H264SwDecNextPicture() to obtain next picture
             * in display order. Function is called until no more images
             * are ready for display */
            while ( H264SwDecNextPicture(decInst, &decPicture, 0) ==
                    H264SWDEC_PIC_RDY )
            {
                DEBUG(("PIC %d, type %s", picDisplayNumber,
                    decPicture.isIdrPicture ? "IDR" : "NON-IDR"));
                if (picDisplayNumber != decPicture.picId)
                    DEBUG((", decoded pic %d", decPicture.picId));
                if (decPicture.nbrOfErrMBs)
                {
                    DEBUG((", concealed %d\n", decPicture.nbrOfErrMBs));
                }
                else
                    DEBUG(("\n"));

                numErrors += decPicture.nbrOfErrMBs;

                /* Increment display number for every displayed picture */
                picDisplayNumber++;

                /*lint -esym(644,decInfo) always initialized if pictures
                 * available for display */

                /* Write output picture to file */
                imageData = (u8*)decPicture.pOutputPicture;
                if (cropDisplay && decInfo.croppingFlag)
                {
                    tmp = CropPicture(tmpImage, imageData,
                        decInfo.picWidth, decInfo.picHeight,
                        &decInfo.cropParams);
                    if (tmp)
                        return -1;
                    // DrawOutput(tmpImage, decInfo.picWidth, decInfo.picHeight);
                    broadwayOnPictureDecoded(tmpImage, decInfo.picWidth, decInfo.picHeight);
                }
                else
                {
                    broadwayOnPictureDecoded(imageData, decInfo.picWidth, decInfo.picHeight);
                    // DrawOutput(imageData, decInfo.picWidth, decInfo.picHeight);
                }
            }

            break;

        case H264SWDEC_STRM_PROCESSED:
        case H264SWDEC_STRM_ERR:
            /* Input stream was decoded but no picture is ready
             * -> Get more data */
            decInput.dataLen = NextPacket(&decInput.pStream);
            break;

        default:
            DEBUG(("FATAL ERROR\n"));
            return -1;

    }
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
u32 NextPacket(u8 **pStrm)
{

    u32 index;
    u32 maxIndex;
    u32 zeroCount;
    u8 *stream;
    u8 byte;
    static u32 prevIndex=0;

    /* For default stream mode all the stream is in first packet */
    if (!packetize && !nalUnitStream)
        return 0;
    
    index = 0;
    stream = *pStrm + prevIndex;
    maxIndex = (u32)(streamStop - stream);

    if (maxIndex == 0)
        return(0);

    /* leading zeros of first NAL unit */
    do
    {
        byte = stream[index++];
    } while (byte != 1 && index < maxIndex);

    /* invalid start code prefix */
    if (index == maxIndex || index < 3)
    {
        DEBUG(("INVALID BYTE STREAM\n"));
        exit(100);
    }

    /* nalUnitStream is without start code prefix */
    if (nalUnitStream)
    {
        stream += index;
        maxIndex -= index;
        index = 0;
    }

    zeroCount = 0;

    /* Search stream for next start code prefix */
    /*lint -e(716) while(1) used consciously */
    while (1)
    {
        byte = stream[index++];
        if (!byte)
            zeroCount++;

        if ( (byte == 0x01) && (zeroCount >= 2) )
        {
            /* Start code prefix has two zeros
             * Third zero is assumed to be leading zero of next packet
             * Fourth and more zeros are assumed to be trailing zeros of this
             * packet */
            if (zeroCount > 3)
            {
                index -= 4;
                zeroCount -= 3;
            }
            else
            {
                index -= zeroCount+1;
                zeroCount = 0;
            }
            break;
        }
        else if (byte)
            zeroCount = 0;

        if (index == maxIndex)
        {
            break;
        }

    }

    /* Store pointer to the beginning of the packet */
    *pStrm = stream;
    prevIndex = index;

    /* nalUnitStream is without trailing zeros */
    if (nalUnitStream)
        index -= zeroCount;

    return(index);

}

/*------------------------------------------------------------------------------

    Function name: CropPicture

    Purpose:
        Perform cropping for picture. Input picture pInImage with dimensions
        picWidth x picHeight is cropped with pCropParams and the resulting
        picture is stored in pOutImage.

------------------------------------------------------------------------------*/
u32 CropPicture(u8 *pOutImage, u8 *pInImage,
    u32 picWidth, u32 picHeight, CropParams *pCropParams)
{

    u32 i, j;
    u32 outWidth, outHeight;
    u8 *pOut, *pIn;

    if (pOutImage == NULL || pInImage == NULL || pCropParams == NULL ||
        !picWidth || !picHeight)
    {
        /* just to prevent lint warning, returning non-zero will result in
         * return without freeing the memory */
        free(pOutImage);
        return(1);
    }

    if ( ((pCropParams->cropLeftOffset + pCropParams->cropOutWidth) >
           picWidth ) ||
         ((pCropParams->cropTopOffset + pCropParams->cropOutHeight) >
           picHeight ) )
    {
        /* just to prevent lint warning, returning non-zero will result in
         * return without freeing the memory */
        free(pOutImage);
        return(1);
    }

    outWidth = pCropParams->cropOutWidth;
    outHeight = pCropParams->cropOutHeight;

    /* Calculate starting pointer for luma */
    pIn = pInImage + pCropParams->cropTopOffset*picWidth +
        pCropParams->cropLeftOffset;
    pOut = pOutImage;

    /* Copy luma pixel values */
    for (i = outHeight; i; i--)
    {
        for (j = outWidth; j; j--)
        {
            *pOut++ = *pIn++;
        }
        pIn += picWidth - outWidth;
    }

    outWidth >>= 1;
    outHeight >>= 1;

    /* Calculate starting pointer for cb */
    pIn = pInImage + picWidth*picHeight +
        pCropParams->cropTopOffset*picWidth/4 + pCropParams->cropLeftOffset/2;

    /* Copy cb pixel values */
    for (i = outHeight; i; i--)
    {
        for (j = outWidth; j; j--)
        {
            *pOut++ = *pIn++;
        }
        pIn += picWidth/2 - outWidth;
    }

    /* Calculate starting pointer for cr */
    pIn = pInImage + 5*picWidth*picHeight/4 +
        pCropParams->cropTopOffset*picWidth/4 + pCropParams->cropLeftOffset/2;

    /* Copy cr pixel values */
    for (i = outHeight; i; i--)
    {
        for (j = outWidth; j; j--)
        {
            *pOut++ = *pIn++;
        }
        pIn += picWidth/2 - outWidth;
    }

    return (0);
}