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

        J2KEncoder(COLOR_FORMAT targetColorFormat, BIT_RATE targetBitRate, PROFILE profile, bool useTiles, RationalNumber fps, int widthUsed, int heightUsed);
        virtual ~J2KEncoder();

        void InitEncoder();

        void EncodeRawFrame(const RawVideoFrame &rawVideoFrame, J2kFrame& encodedFrame);

    private:

        bool EncodeImage(opj_image_t *image, J2kFrame& encodedFrame);

        void SetBroadcastProfile();

        void SetRates(int frameSize);

        void WritePixel8bit(opj_image_t *image, unsigned char r, unsigned char g, unsigned char b, int idx);

        static OPJ_SIZE_T WriteJ2kFrame(void *data, OPJ_SIZE_T bufferSize, void *userData);

        COLOR_FORMAT _targetColorFormat;
        BIT_RATE _targetBitRate;
        PROFILE _profile;
        bool _useTiles;
        RationalNumber _fps;
        opj_image_cmptparm_t* _componentParameter;
        opj_cparameters_t _encodingParameters;
        int _widthUsed;
        int _heightUsed;
        bool _bigEndian;
        OPJ_COLOR_SPACE _colorSpace;
};

#endif // J2KENCODER_H
