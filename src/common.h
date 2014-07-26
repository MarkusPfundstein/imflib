#ifndef IMFLIB_H_INCLUDED
#define IMFLIB_H_INCLUDED

#include <vector>

#include <cstdint>

struct RawVideoFrame
{
    RawVideoFrame() : pixelFormat(0), width(0), height(0), fieldOrder(0) {}

    // danger. inputstreamdecoder does not transfer ownership of data (yet). it only assigns the pointer
    uint8_t *videoData[4];
    int linesize[4];

    int pixelFormat;
    int width;
    int height;

    int fieldOrder;
};

struct J2kFrame
{
    J2kFrame() : data(0) { };

    std::vector<uint8_t> data;
};

struct PCMFrame
{
    PCMFrame() : data(0) { };

    std::vector<uint8_t> data;
};

struct RawAudioFrame
{
    RawAudioFrame() : audioData(nullptr), linesize(0), samples(0) {}
    // danger. inputstreamdecoder does not transfer ownership of data (yet). it only assigns the pointer

    uint8_t *audioData;
    int linesize;
    int samples;
};


struct RationalNumber {
    RationalNumber(int n, int d) : num(n), denum(d) {}
    RationalNumber() : num(0), denum(0) {}
    int num;
    int denum;
};


#endif // IMFLIB_H_INCLUDED
