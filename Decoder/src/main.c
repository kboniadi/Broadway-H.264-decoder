#include "Decoder.h"


void decode(Decoder *dec, u8* stream , u8* typedAr, int length) {
    memcpy(stream, typedAr, length);
    broadwayPlayStream(dec, length);
}

int main() {
    Decoder dec;

    broadwayInit(&dec);
    u8* stream = broadwayCreateStream(&dec, 1024 * 1024);

    
    broadwayExit(&dec);
}