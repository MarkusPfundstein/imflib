#ifndef J2KENCODER_H
#define J2KENCODER_H

extern "C" {
#include <openjpeg-2.1/openjpeg.h>
}

#include "common.h"


class J2KEncoder
{
    public:


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

        explicit J2KEncoder(BIT_RATE targetBitRate, PROFILE profile, bool useTiles, RationalNumber fps, int widthUsed, int heightUsed, bool doMct);

        J2KEncoder(const J2KEncoder& ) = delete;
        J2KEncoder* operator=(const J2KEncoder& ) = delete;

        virtual ~J2KEncoder();

        void InitEncoder();

        void EncodeRawFrame(const RawVideoFrame &rawVideoFrame, J2kFrame& encodedFrame);

    private:
        bool EncodeImage(opj_image_t *image, J2kFrame& encodedFrame);

        void SetBroadcastProfile();

        void SetRates(int frameSize);

        void FillImagePacked(opj_image_t *image, const RawVideoFrame &rawFrame);
        void FillImagePlanar(opj_image_t *image, const RawVideoFrame &rawFrame);

        static OPJ_SIZE_T WriteJ2kFrame(void *data, OPJ_SIZE_T bufferSize, void *userData);

        BIT_RATE _targetBitDepth;
        PROFILE _profile;
        bool _useTiles;
        RationalNumber _fps;
        opj_image_cmptparm_t* _componentParameter;
        int _widthUsed;
        int _heightUsed;
        opj_cparameters_t _encodingParameters;
        bool _bigEndian;
        OPJ_COLOR_SPACE _colorSpace;
        bool _mct;
};

#endif // J2KENCODER_H
