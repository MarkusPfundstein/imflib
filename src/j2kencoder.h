#ifndef J2KENCODER_H
#define J2KENCODER_H

extern "C" {
#include <openjpeg-2.1/openjpeg.h>
}
// forward declarations
struct RawVideoFrame;


struct J2kFrame
{

};

class J2KEncoder
{
    public:
        J2KEncoder();
        virtual ~J2KEncoder();

        bool EncodeRawFrame(RawVideoFrame &rawVideoFrame, J2kFrame& encodedFrame);

    private:
        bool EncodeImage(opj_image_t *image, J2kFrame& encodedFrame, opj_cparameters_t &parameters);
};

#endif // J2KENCODER_H
