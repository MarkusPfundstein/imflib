#ifndef J2KENCODER_H
#define J2KENCODER_H

extern "C" {
#include <openjpeg.h>
}

#include "common.h"


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
            BR_12bit = 12,
            BR_16bit = 16
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

        explicit J2KEncoder(COLOR_FORMAT targetColorFormat, BIT_RATE targetBitRate, PROFILE profile, bool useTiles, RationalNumber fps, int widthUsed, int heightUsed);

        J2KEncoder(const J2KEncoder& ) = delete;
        J2KEncoder* operator=(const J2KEncoder& ) = delete;

        virtual ~J2KEncoder();

        void InitEncoder();

        void EncodeRawFrame(const RawVideoFrame &rawVideoFrame, J2kFrame& encodedFrame);

    private:
        struct ColorComponent
        {
            unsigned char c1, c2, c3;
        };

        bool EncodeImage(opj_image_t *image, J2kFrame& encodedFrame);

        void SetBroadcastProfile();

        void SetRates(int frameSize);

        void RGB24toYUV24(ColorComponent &rgb);

        static OPJ_SIZE_T WriteJ2kFrame(void *data, OPJ_SIZE_T bufferSize, void *userData);

        COLOR_FORMAT _targetColorFormat;
        BIT_RATE _targetBitRate;
        PROFILE _profile;
        bool _useTiles;
        RationalNumber _fps;
        opj_image_cmptparm_t* _componentParameter;
        int _widthUsed;
        int _heightUsed;
        opj_cparameters_t _encodingParameters;
        bool _bigEndian;
        OPJ_COLOR_SPACE _colorSpace;
};

#endif // J2KENCODER_H
