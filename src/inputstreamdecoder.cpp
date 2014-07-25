#include "inputstreamdecoder.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/samplefmt.h>
//#include <libavutil/timestamp.h>
#ifdef __cplusplus
}
#endif

#include <iostream>
#include <cstring>
#include <mutex>
#include <cmath>

InputStreamDecoder::InputStreamDecoder(const std::string &file, int bitDepth, int audioRate)
    : _formatContext(nullptr), _videoStreamContext(), _audioStreams(), _subtitleStreams(), _swsContext(nullptr), _targetFormat(-1), _audioRate(audioRate)
{
    if (bitDepth == 8 || bitDepth == 10 || bitDepth == 12) {
        _targetFormat = PIX_FMT_RGB24;
    } else if (bitDepth <= 16) {
        _targetFormat = PIX_FMT_RGB48;
    }
    OpenFile(file);
}

InputStreamDecoder::~InputStreamDecoder()
{
    CloseFile();
    if (_swsContext) {
        sws_freeContext(_swsContext);
    }
    for (std::shared_ptr<AudioStreamContext>& ctx : _audioStreams) {
        if (ctx->resampleContext) {
            swr_free(&ctx->resampleContext);
        }
    }
}

void InputStreamDecoder::CloseFile()
{
    if (_formatContext) {
        avformat_close_input(&_formatContext);
    }
}

void InputStreamDecoder::OpenFile(const std::string& file)
{
    if (avformat_open_input(&_formatContext, file.c_str(), nullptr, nullptr) != 0) {
        throw std::runtime_error("error opening file");
    }

    if (avformat_find_stream_info(_formatContext, nullptr) < 0) {
        throw std::runtime_error("error reading stream info from file");
    }

    for (unsigned int i = 0; i < _formatContext->nb_streams; ++i) {

        // first , lets get codec and codec context
        AVStream *stream = _formatContext->streams[i];

        AVCodec *codec = avcodec_find_decoder(stream->codec->codec_id);
        if (codec == nullptr) {
            throw std::runtime_error("couldn't find codec for stream: " + stream->index);
        }

        CodecContextPtr codecContext(avcodec_alloc_context3(codec),
                                     [](AVCodecContext* c) {
                                        if (c->extradata != nullptr) {
                                            delete [] c->extradata;
                                        }
                                        avcodec_close(c);
                                        av_free(c);
                                    });

        // copy extradata into codec context. the +1 is necessary for whatever reason.
        codecContext->extradata = new uint8_t[stream->codec->extradata_size + 1];
        std::copy(stream->codec->extradata, stream->codec->extradata + stream->codec->extradata_size, codecContext->extradata);

        codecContext->extradata_size = stream->codec->extradata_size;

        // BUG: MUST BE SET BEFORE avcodec_open2 IFF sound input is PCM
        codecContext->channels = 2;

        if (avcodec_open2(codecContext.get(), codec, nullptr) < 0) {
            throw std::runtime_error("[InputStreamDecoder] Could not open codec");
        }

        switch (stream->codec->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                if (_videoStreamContext.context) {
                    std::cout << "second video stream detected. this is not implemented yet [skip]" << std::endl;
                    continue;
                }
                std::cout << "video stream detected: " << stream->index << std::endl;

                AddVideoStream(stream, codecContext);


                break;
            case AVMEDIA_TYPE_AUDIO:
                std::cout << "audio stream detected: " << stream->index << std::endl;

                AddAudioStream(stream, codecContext);

                break;
            case AVMEDIA_TYPE_SUBTITLE:
                std::cout << "subtitle stream detected: " << stream->index << std::endl;
                //_subtitleStreams.push_back(std::make_tuple(stream, codecContext));
                break;
            case AVMEDIA_TYPE_DATA:
            case AVMEDIA_TYPE_ATTACHMENT:
            case AVMEDIA_TYPE_NB:
            case AVMEDIA_TYPE_UNKNOWN:
            default:
                std::cout << "invalid stream detected: " << stream->index << std::endl;
        }
    }

    // unlikely
    if ((_videoStreamContext.context == nullptr) && _audioStreams.empty() && _subtitleStreams.empty()) {
        throw std::runtime_error("no streams detected");
    }
}

void InputStreamDecoder::AddVideoStream(AVStream *stream, const CodecContextPtr &context)
{
    if (stream->codec->codec){
        std::cout << "codec: " << stream->codec->codec->name << std::endl;
    } else {
        std::cout << "couldn't detect codec name" << std::endl;
    }
    std::cout << "pix_fmt: " << av_get_pix_fmt_name(stream->codec->pix_fmt) << std::endl;
    std::cout << "size: " << stream->codec->width << ":" << stream->codec->height << std::endl;

    _videoStreamContext.context = context;
    _videoStreamContext.stream = stream;

    // store data for later in raw video frame
    _videoStreamContext.videoFrame.width = stream->codec->width;
    _videoStreamContext.videoFrame.height = stream->codec->height;
    _videoStreamContext.videoFrame.pixelFormat = stream->codec->pix_fmt;
    _videoStreamContext.videoFrame.fieldOrder = stream->codec->field_order;

    _swsContext = sws_getContext(stream->codec->width,
                                 stream->codec->height,
                                 (PixelFormat) stream->codec->pix_fmt,
                                 stream->codec->width,
                                 stream->codec->height,
                                 (PixelFormat) _targetFormat,
                                 SWS_BILINEAR,
                                 nullptr,
                                 nullptr,
                                 nullptr);
    if (_swsContext == nullptr) {
        throw std::runtime_error("Error while calling sws_getContext");
    }
}

void InputStreamDecoder::AddAudioStream(AVStream *stream, const CodecContextPtr &context)
{
    std::cout << "sample_rate: " << stream->codec->sample_rate << std::endl;
    std::cout << "channels: " << stream->codec->channels << std::endl;
    std::cout << "format index: " << context->sample_fmt << std::endl;
    std::cout << "source rate: " << stream->codec->sample_rate << std::endl;


    SwrContext *resampleContext = swr_alloc();
    if (resampleContext == nullptr) {
        throw std::runtime_error("[InputStreamDecoder] error allocating resample context");
    }

    av_opt_set_int(resampleContext,        "in_channel_layout",  av_get_default_channel_layout(context->channels), 0);
    av_opt_set_int(resampleContext,        "in_sample_rate",     stream->codec->sample_rate, 0);
    av_opt_set_sample_fmt(resampleContext, "in_sample_fmt",      context->sample_fmt, 0);
    av_opt_set_int(resampleContext,        "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(resampleContext,        "out_sample_rate",    _audioRate, 0);
    av_opt_set_sample_fmt(resampleContext, "out_sample_fmt",     AV_SAMPLE_FMT_S32, 0);

    if (swr_init(resampleContext) != 0) {
        throw std::runtime_error("[InputStreamDecoder] error initializing resample context");
    }

    std::shared_ptr<AudioStreamContext> ctx(new AudioStreamContext(stream, context, resampleContext));

    ctx->wav.sampleRate = _audioRate;
    ctx->wav.sampleSize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S32);
    ctx->wav.channels = 2;

    _audioStreams.push_back(ctx);
}

bool InputStreamDecoder::HasVideoTrack() const
{
    return _videoStreamContext.stream != nullptr;
}

int InputStreamDecoder::GetNumberAudioTracks() const
{
    return _audioStreams.size();
}

RationalNumber InputStreamDecoder::GetFrameRate() const
{
    if (_videoStreamContext.stream == nullptr) {
        return RationalNumber(0, 0);
    }
    AVRational rat = _videoStreamContext.stream->r_frame_rate;
    RationalNumber n;
    av_reduce(&n.num, &n.denum, rat.num, rat.den, 1024*1024);
    return n;
}

RationalNumber InputStreamDecoder::GetAspectRatio() const
{
    AVRational *sar = nullptr;
    AVCodecContext *context = _videoStreamContext.context.get();
    AVStream *stream = _videoStreamContext.stream;
    if (stream == nullptr || context == nullptr) {
        return RationalNumber(0, 0);
    }
    if (context->sample_aspect_ratio.num) {
        std::cout << "here" << std::endl;
        sar = &context->sample_aspect_ratio;
    } else if (stream->sample_aspect_ratio.num) {
        std::cout << "or here" << std::endl;
        sar = &stream->sample_aspect_ratio;
    }
    AVRational source;
    if (sar == nullptr) {
        // brute force divide
        source.num = stream->codec->width;
        source.den = stream->codec->height;
    } else {
        source.num = sar->num * stream->codec->width;
        source.den = sar->den * stream->codec->height;
    }
    AVRational displayAspectRatio;
    av_reduce(&displayAspectRatio.num, &displayAspectRatio.den, source.num, source.den, 1024*1024);
    return RationalNumber(displayAspectRatio.num, displayAspectRatio.den);
}

int InputStreamDecoder::GetVideoWidth() const
{
    AVStream *stream = _videoStreamContext.stream;
    if (stream == nullptr) {
        return 0;
    }
    return stream->codec->width;
}

int InputStreamDecoder::GetVideoHeight() const
{
    AVStream *stream = _videoStreamContext.stream;
    if (stream == nullptr) {
        return 0;
    }
    return stream->codec->height;
}

void InputStreamDecoder::Decode(GotVideoFrameCallbackFunction videoCallback, GotAudioFrameCallbackFunction audioCallback)
{
    AVPacket packet;
    packet.data = nullptr;
    packet.size = 0;

    bool noError = true;

    // memory managed AVFrame.
    std::shared_ptr<AVFrame> decodedFrame(avcodec_alloc_frame(), &av_free);

    // decoding loop
    while (noError) {

        // NOTE: FROM HERE ON av_free_packet must be called somewhere.
        // so dont just leave the loop
        if (av_read_frame(_formatContext, &packet) < 0) {
            std::cout << "done" << std::endl;
            break;
        }

        // store reference to original packet. for freeing memory later on. because
        // next loop will adjust data offset
        AVPacket origPacket = packet;

        // decode packet.
        do {
            int gotFrame = 0;
            int audioStreamIndex;
            FRAME_TYPE frameType;
            int processedBytes = DecodePacket(packet, *decodedFrame, gotFrame, frameType, audioStreamIndex);
            if (processedBytes < 0) {
                noError = false;
                break;
            }
            // awesome. we got a full frame
            if (gotFrame) {
                bool success = HandleFrame(*decodedFrame, frameType, videoCallback, audioCallback, audioStreamIndex);
                if (success == false) {
                    noError = false;
                    break;
                }
            }

            // packet could still have data, lets adjust data pointer and size
            packet.data += processedBytes;
            packet.size -= processedBytes;
        } while (packet.size > 0);

        av_free_packet(&origPacket);

    }

    // flush remaining data
    packet.data = nullptr;
    packet.size = 0;

    int audioStreamIndex;
    int gotFrame = 0;
    FRAME_TYPE frameType;

    do {
        DecodePacket(packet, *decodedFrame, gotFrame, frameType, audioStreamIndex);

        // VERY unlikely. but why not.
        if (gotFrame) {
            HandleFrame(*decodedFrame, frameType, videoCallback, audioCallback, audioStreamIndex);
        }
    } while (gotFrame);

    for (unsigned int i = 0; i < _audioStreams.size(); ++i) {
        HandleDelayedAudioFrames(i, audioCallback);
    }
}

bool InputStreamDecoder::HandleFrame(AVFrame& decodedFrame, FRAME_TYPE frameType, GotVideoFrameCallbackFunction videoCallback,
                                      GotAudioFrameCallbackFunction audioCallback, int audioStreamIndex)
{
    bool success = true;
    AVPicture pic;

    switch (frameType) {
        case FRAME_TYPE::VIDEO:

            avpicture_alloc(&pic, (PixelFormat)_targetFormat, decodedFrame.width, decodedFrame.height);
            sws_scale(_swsContext, decodedFrame.data, decodedFrame.linesize, 0, decodedFrame.height, pic.data, pic.linesize);

            // TO-DO: Give owhership over data to _videoStreamContext
            _videoStreamContext.videoFrame.pixelFormat = _targetFormat;
            for (int i = 0; i < 4; ++i) {
                _videoStreamContext.videoFrame.videoData[i] = pic.data[i];
                _videoStreamContext.videoFrame.linesize[i] = pic.linesize[i];
            }

            // pass RawVideoFrame to user so that she can do what she wants.
            try {
                success = videoCallback(_videoStreamContext.videoFrame);
                avpicture_free(&pic);
            } catch (...) {
                avpicture_free(&pic);
                throw;
            }
            break;
        case FRAME_TYPE::AUDIO:

            success = HandleAudioFrame(decodedFrame, audioCallback, audioStreamIndex);

            break;
        case FRAME_TYPE::SUBTITLES:
        case FRAME_TYPE::UNKNOWN:
        default:
            // maybe no throwing here. time will tell...
            throw new std::runtime_error("invalid frame type");
    }

    return success;
}

bool InputStreamDecoder::HandleAudioFrame(AVFrame& decodedFrame, GotAudioFrameCallbackFunction audioCallback, int audioStreamIndex)
{
    bool success = true;
    try {
        std::shared_ptr<AudioStreamContext> ctx = _audioStreams[audioStreamIndex];
        SwrContext *swrContext = ctx->resampleContext;

        unsigned char **out;
        int outLinesize;
        int outSamples = av_rescale_rnd(decodedFrame.nb_samples, ctx->wav.sampleRate, ctx->stream->codec->sample_rate, AV_ROUND_UP);
        av_samples_alloc_array_and_samples(&out,
                                           &outLinesize,
                                           ctx->wav.channels,
                                           outSamples,
                                           AV_SAMPLE_FMT_S32,
                                           0);

        int newSamples = swr_convert(swrContext,
                                      out,
                                      outSamples,
                                      (const uint8_t **)decodedFrame.extended_data,
                                      decodedFrame.nb_samples);
        if (newSamples < 0) {
            std::cout << "Error while converting" << std::endl;
            av_freep(out);
            return false;
        }
        int bufferSize = av_samples_get_buffer_size(&outLinesize, ctx->wav.channels, newSamples, AV_SAMPLE_FMT_S32, 1);

        PCMFrame frame;
        std::copy(out[0], out[0] + bufferSize, std::back_inserter(frame.data));
        frame.samples = newSamples;
        audioCallback(frame, audioStreamIndex);

        if (out) {
            av_freep(&out[0]);
        }
        av_freep(&out);


    } catch (...) {
        throw;
    }
    return success;
}

bool InputStreamDecoder::HandleDelayedAudioFrames(int audioStreamIndex, GotAudioFrameCallbackFunction audioCallback)
{
    int newSamples = 0;
    do {
        std::cout << "HANDLE DELAYED RESAMPLE FRAMES" << std::endl;
        std::shared_ptr<AudioStreamContext> ctx = _audioStreams[audioStreamIndex];
        SwrContext *swrContext = ctx->resampleContext;

        unsigned char **out;
        int outLinesize;
        int outSamples = av_rescale_rnd(1152, ctx->wav.sampleRate, ctx->stream->codec->sample_rate, AV_ROUND_UP);
        av_samples_alloc_array_and_samples(&out,
                                           &outLinesize,
                                           ctx->wav.channels,
                                           outSamples,
                                           AV_SAMPLE_FMT_S32,
                                           0);

        newSamples = swr_convert(swrContext,
                                 out,
                                 outSamples,
                                 nullptr,
                                 0);
        if (newSamples < 0) {
            std::cout << "Error while converting" << std::endl;
            av_freep(out);
            return false;
        }
        int bufferSize = av_samples_get_buffer_size(&outLinesize, ctx->wav.channels, newSamples, AV_SAMPLE_FMT_S32, 1);

        PCMFrame frame;
        std::copy(out[0], out[0] + bufferSize, std::back_inserter(frame.data));
        frame.samples = newSamples;
        audioCallback(frame, audioStreamIndex);

        if (out) {
            av_freep(&out[0]);
        }
        av_freep(&out);
    } while (newSamples > 0);

    return true;
}

int InputStreamDecoder::DecodePacket(AVPacket& packet, AVFrame& decodedFrame, int &gotFrame, FRAME_TYPE &frameType, int &index)
{
    AVStream* stream = _videoStreamContext.stream;

    index = -1;
    // there could be no video stream
    if (stream) {
        AVCodecContext *codecContext = _videoStreamContext.context.get();
        if (stream->index == packet.stream_index) {
            int processedBytes = avcodec_decode_video2(codecContext, &decodedFrame, &gotFrame, &packet);
            std::cout << "[VIDEO] processedBytes: " << processedBytes << std::endl;
            frameType = FRAME_TYPE::VIDEO;
            return processedBytes;
        }
    }

    // search for audio stream
    int j = 0;
    for (auto i = _audioStreams.begin(); i != _audioStreams.end(); ++i) {
        std::shared_ptr<AudioStreamContext> ctx = *i;
        AVStream* audioStream = ctx->stream;
        AVCodecContext *audioCodecContext = ctx->context.get();
        if (audioStream->index == packet.stream_index) {
            int processedBytes = avcodec_decode_audio4(audioCodecContext, &decodedFrame, &gotFrame, &packet);
            std::cout << "[AUDIO] processedBytes: " << processedBytes << std::endl;
            frameType = FRAME_TYPE::AUDIO;
            index = j++;
            return processedBytes;
        }
    }

    // very unlikely
    frameType = FRAME_TYPE::UNKNOWN;

    return -1;
}

void InputStreamDecoder::RegisterAVFormat()
{
    static std::once_flag initFlag;
    std::call_once(initFlag, []() { av_register_all(); });
}
