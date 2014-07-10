#ifndef RAWVIDEOFRAME_H
#define RAWVIDEOFRAME_H


struct RawVideoFrame
{
    RawVideoFrame();
    ~RawVideoFrame();

    uint8_t *videoData;
    int bufferSize;

    int pixelFormat;
    int width;
    int height;
}

#endif // RAWVIDEOFRAME_H
