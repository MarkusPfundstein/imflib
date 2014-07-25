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
#include <memory>
#include <vector>

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

    _codecContext->channels = 2;
    _codecContext->bit_rate = _sampleRate * _codecContext->channels * 32;
    _codecContext->sample_rate = (int)_sampleRate;
    _codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
    _codecContext->sample_fmt = AV_SAMPLE_FMT_S32;  // must be 32 for pcm_s24le

    //AVRational timeBase = {90000, 1};

    //_codecContext->time_base = timeBase;

    if (avcodec_open2(_codecContext, codec, nullptr) < 0) {
        throw std::runtime_error("[PCM] Couldn't open codec");
    }

    std::cout << "[PCM] Encoder initialized" << std::endl;
}

void PCMEncoder::EncodeRawFrame(const PCMFrame& rawFrame, PCMFrame &pcmFrame)
{
    if (rawFrame.samples == 0) {
        std::cout << "[PCM] got empty frame" << std::endl;
        return;
    }
    std::shared_ptr<AVFrame> frame(av_frame_alloc(), &av_free);
    frame->nb_samples     = rawFrame.samples;
    frame->channel_layout = _codecContext->channel_layout;
    frame->format         = _codecContext->sample_fmt;
    frame->sample_rate    = _codecContext->sample_rate;

    int bufferSize = av_samples_get_buffer_size(nullptr, _codecContext->channels, rawFrame.samples, _codecContext->sample_fmt, 0);

    uint8_t *samples = (uint8_t*)av_malloc(bufferSize);

    avcodec_fill_audio_frame(frame.get(), _codecContext->channels, _codecContext->sample_fmt, (const uint8_t*)samples, bufferSize, 0);

    for (unsigned int i = 0; i < rawFrame.data.size(); ++i) {
        samples[i] = rawFrame.data[i];
    }

    AVPacket packet;
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;

    int gotPacket = 0;
    if (avcodec_encode_audio2(_codecContext, &packet, frame.get(), &gotPacket) < 0) {
        av_free_packet(&packet);
        av_free(samples);
        throw std::runtime_error("[PCM] error encoding audio");
    }

    static std::ofstream of("/home/markus/Documents/IMF/TestFiles/OUT.raw", std::ios::binary | std::ios::out);
    if (gotPacket) {
        std::cout << "write " << packet.size << std::endl;
        pcmFrame.data.reserve(packet.size);
        std::copy((uint8_t*)packet.data, (uint8_t*)packet.data + packet.size, std::back_inserter(pcmFrame.data));
        av_free_packet(&packet);

        of.write((const char*)&pcmFrame.data[0], pcmFrame.data.size());
    }
    av_free(samples);
}
