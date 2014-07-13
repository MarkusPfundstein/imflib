#ifndef RAWVIDEOFRAME_H
#define RAWVIDEOFRAME_H

#include <cstdint>

struct RawVideoFrame
{
    RawVideoFrame() : pixelFormat(0), width(0), height(0), fieldOrder(0) {}

    uint8_t *videoData[4];
    int linesize[4];

    int pixelFormat;
    int width;
    int height;

    int fieldOrder;
};


#endif // RAWVIDEOFRAME_H
