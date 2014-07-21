#ifndef J2KENCODER_H
#define J2KENCODER_H

extern "C" {
#include <openjpeg-2.1/openjpeg.h>
}
// forward declarations
struct RawVideoFrame;
struct J2kFrame;
#include "rationalnumber.h"


class J2KEncoder
{
    public:
        enum COLOR_FORMAT {
            CF_RGB444 = 0,
            CF_YUV444 = 1,
            CF_YUV422 = 2
        };

        enum BIT_RATE {
            BR_8bit = 8,
            BR_10bit = 10,
            BR_12bit = 12
        };

        enum PROFILE {
            BCP_ST_1 = 1,
            BCP_ST_2,
            BCP_ST_3,
            BCP_ST_4,
            BCP_ST_5,
            BCP_MT_6,
            BCP_MT_7
        };

        J2KEncoder(COLOR_FORMAT targetColorFormat, BIT_RATE targetBitRate, PROFILE profile, bool useTiles);
        virtual ~J2KEncoder();

        void EncodeRawFrame(const RawVideoFrame &rawVideoFrame, J2kFrame& encodedFrame, RationalNumber fps);

    private:
        bool EncodeImage(opj_image_t *image, J2kFrame& encodedFrame, opj_cparameters_t &parameters, int widthUsed, int heightUsed);

        void SetBroadcastProfile(opj_cparameters_t &parameters, PROFILE profile, int widthUsed, int heightUsed, RationalNumber fps);

        void SetRates(PROFILE profile, RationalNumber targetFps, opj_cparameters_t &parameters, int frameSize);

        static OPJ_SIZE_T WriteJ2kFrame(void *data, OPJ_SIZE_T bufferSize, void *userData);

        COLOR_FORMAT _targetColorFormat;
        BIT_RATE _targetBitRate;
        PROFILE _profile;
        bool _useTiles;
};

#endif // J2KENCODER_H
