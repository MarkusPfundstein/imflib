#include "pcmencoder.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

PCMEncoder::PCMEncoder()
{
    //ctor
}

PCMEncoder::~PCMEncoder()
{
    //dtor
}

void PCMEncoder::InitEncoder()
{

}

void PCMEncoder::EncodeRawFrame(const AVFrame& rawFrame)
{

}
