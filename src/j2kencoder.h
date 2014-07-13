#ifndef J2KENCODER_H
#define J2KENCODER_H

#include <vector>

extern "C" {
#include <openjpeg-2.1/openjpeg.h>
}
// forward declarations
struct RawVideoFrame;

struct J2kFrame
{
    J2kFrame() : data(0) { };

    std::vector<uint8_t> data;
};

class J2KEncoder
{
    public:
        J2KEncoder();
        virtual ~J2KEncoder();

        bool EncodeRawFrame(RawVideoFrame &rawVideoFrame, J2kFrame& encodedFrame);

    private:
        bool EncodeImage(opj_image_t *image, J2kFrame& encodedFrame, opj_cparameters_t &parameters);

        static OPJ_SIZE_T WriteJ2kFrame(void *data, OPJ_SIZE_T bufferSize, void *userData);
};

#endif // J2KENCODER_H
