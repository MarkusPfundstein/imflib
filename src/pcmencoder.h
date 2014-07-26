#ifndef PCMENCODER_H
#define PCMENCODER_H

struct AVFrame;
struct SwrContext;
struct AVCodecContext;

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

        PCMEncoder(SAMPLE_RATE sampleRate, int channels, int channelLayout);
        ~PCMEncoder();

        PCMEncoder(const PCMEncoder&) = delete;
        PCMEncoder *operator=(const PCMEncoder&) = delete;

        void InitEncoder();

        void EncodeRawFrame(const RawAudioFrame &rawFrame, std::vector<uint8_t> &outData);

    protected:
    private:

        AVCodecContext *_codecContext;
        SAMPLE_RATE _sampleRate;

        int _channels;
        int _channelLayoutIndex;
};

#endif // PCMENCODER_H
