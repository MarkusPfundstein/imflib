#ifndef PCMENCODER_H
#define PCMENCODER_H

struct AVFrame;
struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;

#include <string>
#include <stdint.h>
#include "common.h"

class PCMEncoder
{
    public:
        enum SAMPLE_RATE {
            SR_48000 = 48000,
            SR_96000 = 96000
        };

        PCMEncoder(SAMPLE_RATE sampleRate);
        ~PCMEncoder();

        PCMEncoder(const PCMEncoder&) = delete;
        PCMEncoder *operator=(const PCMEncoder&) = delete;

        void InitEncoder();

        void EncodeRawFrame(const AVFrame &rawFrame, PCMFrame& pcmFrame);

    protected:
    private:

        void InitConvertedInputSamples(uint8_t ***convertedInputSamples, int frameSize);

        AVCodecContext *_codecContext;
        SAMPLE_RATE _sampleRate;
};

#endif // PCMENCODER_H
