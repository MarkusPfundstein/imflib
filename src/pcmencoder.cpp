#include "pcmencoder.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/avstring.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
}
#endif

#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <fstream>

PCMEncoder::PCMEncoder(SAMPLE_RATE sampleRate)
    :
    _codecContext(nullptr),
    _sampleRate(sampleRate)
{
    //ctor
}

PCMEncoder::~PCMEncoder()
{
    if (_codecContext != nullptr) {
        avcodec_close(_codecContext);
        av_free(_codecContext);
    }

}

void PCMEncoder::InitEncoder()
{
    AVCodec *codec = avcodec_find_encoder(CODEC_ID_PCM_S24LE);
    if (codec == nullptr) {
        throw std::runtime_error("[PCM] pcm_24le codec not found");
    }

    _codecContext = avcodec_alloc_context3(codec);
    if (_codecContext == nullptr) {
        throw std::runtime_error("[PCM] error getting codec context");
    }

    _codecContext->bit_rate = (_sampleRate == SR_48000 ? 2024 : 4086) * 1024;
    _codecContext->sample_rate = (int)_sampleRate;
    _codecContext->channels = 2;
    _codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
    _codecContext->sample_fmt = AV_SAMPLE_FMT_S32;  // must be 32 for pcm_s24le

    AVRational timeBase = {90000, 1};

    _codecContext->time_base = timeBase;

    if (avcodec_open2(_codecContext, codec, nullptr) < 0) {
        throw std::runtime_error("[PCM] Couldn't open codec");
    }

    std::cout << "[PCM] Encoder initialized" << std::endl;
}

void PCMEncoder::EncodeRawFrame(const AVFrame& rawFrame, PCMFrame &pcmFrame)
{
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;

    int gotPacket = 0;
    if (avcodec_encode_audio2(_codecContext, &packet, &rawFrame, &gotPacket) < 0) {
        av_free_packet(&packet);
        throw std::runtime_error("[PCM] error encoding audio");
    }

    //static std::ofstream of("/home/markus/Documents/IMF/TestFiles/OUT.pcm");

    if (gotPacket) {
        //pcmFrame.data.reserve(packet.size);
        //std::copy((uint8_t*)packet.data, (uint8_t*)packet.data + packet.size, std::back_inserter(pcmFrame.data));
        //av_free_packet(&packet);

        //of.write((char*)pcmFrame.data[0], (int)pcmFrame.data.size());
    }
}
