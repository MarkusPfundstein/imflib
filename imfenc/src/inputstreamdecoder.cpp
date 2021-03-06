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

InputStreamDecoder::InputStreamDecoder(const std::string &file, int bitDepth, COLOR_FORMAT targetColorFormat, int audioRate, bool extractAudio)
    : _formatContext(nullptr), _videoStreamContext(), _audioStreams(), _subtitleStreams(), _swsContext(nullptr), _targetVideoPixelFormat(-1), _targetAudioSampleRate(audioRate), _doneCallback(), _extractAudio(extractAudio)
{

    if (bitDepth == 8) {
        if (targetColorFormat == CF_RGB444) {
            _targetVideoPixelFormat = AV_PIX_FMT_RGB24;
        } else if (targetColorFormat == CF_YUV444) {
            _targetVideoPixelFormat = AV_PIX_FMT_YUV444P;
        } else if (targetColorFormat == CF_YUV422) {
            _targetVideoPixelFormat = AV_PIX_FMT_YUV422P;
        }
    } else if (bitDepth == 10) {
        if (targetColorFormat == CF_RGB444) {
            _targetVideoPixelFormat = AV_PIX_FMT_GBRP10;
        } else if (targetColorFormat == CF_YUV444) {
            _targetVideoPixelFormat = AV_PIX_FMT_YUV444P10;
        } else if (targetColorFormat == CF_YUV422) {
            _targetVideoPixelFormat = AV_PIX_FMT_YUV422P10;
        }
    } else if (bitDepth == 12) {
        if (targetColorFormat == CF_RGB444) {
            _targetVideoPixelFormat = AV_PIX_FMT_GBRP12;
        } else if (targetColorFormat == CF_YUV444) {
            _targetVideoPixelFormat = AV_PIX_FMT_YUV444P12;
        } else if (targetColorFormat == CF_YUV422) {
            _targetVideoPixelFormat = AV_PIX_FMT_YUV422P12;
        }
    }

    if (_targetVideoPixelFormat == -1) {
        throw std::runtime_error("unsupported pixel format");
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
        throw std::runtime_error("[InputStreamDecoder] error opening file");
    }

    if (avformat_find_stream_info(_formatContext, nullptr) < 0) {
        throw std::runtime_error("[InputStreamDecoder] error reading stream info from file");
    }

    for (unsigned int i = 0; i < _formatContext->nb_streams; ++i) {
        // first , lets get codec and codec context
        AVStream *stream = _formatContext->streams[i];

        AVCodec *codec = avcodec_find_decoder(stream->codec->codec_id);
        if (codec == nullptr) {
            //throw std::runtime_error("couldn't find codec for stream: " + stream->index);
            continue;
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


        if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            // must be set before avcodec_open2 if sound input is PCM
            codecContext->channels = stream->codec->channels;
        }

        if (avcodec_open2(codecContext.get(), codec, nullptr) < 0) {
            throw std::runtime_error("[InputStreamDecoder] Could not open codec");
        }

        switch (stream->codec->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                if (_videoStreamContext.context) {
                    std::cerr << "[InputStreamDecoder] second video stream detected. this is not implemented yet [skip]" << std::endl;
                    continue;
                }
                std::cout << "[InputStreamDecoder] Video stream detected: " << stream->index << std::endl;

                AddVideoStream(stream, codecContext);


                break;
            case AVMEDIA_TYPE_AUDIO:
                std::cout << "[InputStreamDecoder] Audio stream detected: " << stream->index << std::endl;

                AddAudioStream(stream, codecContext);

                break;
            case AVMEDIA_TYPE_SUBTITLE:
                std::cout << "[InputStreamDecoder] Subtitle stream detected: " << stream->index << std::endl;
                //_subtitleStreams.push_back(std::make_tuple(stream, codecContext));
                break;
            case AVMEDIA_TYPE_DATA:
            case AVMEDIA_TYPE_ATTACHMENT:
            case AVMEDIA_TYPE_NB:
            case AVMEDIA_TYPE_UNKNOWN:
            default:
                std::cerr << "[InputStreamDecoder] Invalid stream detected: " << stream->index << std::endl;
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
        std::cout << "[InputStreamDecoder] Codec: " << stream->codec->codec->name << std::endl;
    } else {
        std::cout << "[InputStreamDecoder] Couldn't detect codec name" << std::endl;
    }
    std::cout << "[InputStreamDecoder] Size: " << stream->codec->width << ":" << stream->codec->height << std::endl;

    _videoStreamContext.context = context;
    _videoStreamContext.stream = stream;
    // important for prores
    _videoStreamContext.context->width = stream->codec->width;
    _videoStreamContext.context->height = stream->codec->height;

    //int flags = SWS_FULL_CHR_H_INT | SWS_FULL_CHR_H_INP | SWS_ACCURATE_RND | SWS_BITEXACT | SWS_POINT;

    _swsContext = sws_getContext(stream->codec->width,
                                 stream->codec->height,
                                 (PixelFormat) stream->codec->pix_fmt,
                                 stream->codec->width,
                                 stream->codec->height,
                                 (PixelFormat) _targetVideoPixelFormat,
                                 SWS_BILINEAR | SWS_ACCURATE_RND | SWS_BITEXACT | SWS_PRINT_INFO /*flags*/,
                                 nullptr,
                                 nullptr,
                                 nullptr);
    if (_swsContext == nullptr) {
        throw std::runtime_error("Error while calling sws_getContext");
    }
}

void InputStreamDecoder::AddAudioStream(AVStream *stream, const CodecContextPtr &context)
{
    std::cout << "[InputStreamDecoder] sample_rate: " << stream->codec->sample_rate << std::endl;
    std::cout << "[InputStreamDecoder] channels: " << stream->codec->channels << std::endl;
    std::cout << "[InputStreamDecoder] format index: " << context->sample_fmt << std::endl;
    std::cout << "[InputStreamDecoder] source rate: " << stream->codec->sample_rate << std::endl;

    SwrContext *resampleContext = swr_alloc();
    if (resampleContext == nullptr) {
        throw std::runtime_error("[InputStreamDecoder] error allocating resample context");
    }

    int channelLayout = stream->codec->channel_layout;
    if (channelLayout == 0) {
        channelLayout = av_get_default_channel_layout(context->channels);
    }

    av_opt_set_int(resampleContext,        "in_channel_layout",  channelLayout/*av_get_default_channel_layout(context->channels)*/, 0);
    av_opt_set_int(resampleContext,        "in_sample_rate",     stream->codec->sample_rate, 0);
    av_opt_set_sample_fmt(resampleContext, "in_sample_fmt",      context->sample_fmt, 0);
    av_opt_set_int(resampleContext,        "out_channel_layout", channelLayout, 0);
    av_opt_set_int(resampleContext,        "out_sample_rate",    _targetAudioSampleRate, 0);
    av_opt_set_sample_fmt(resampleContext, "out_sample_fmt",     AV_SAMPLE_FMT_S32, 0);

    if (swr_init(resampleContext) != 0) {
        throw std::runtime_error("[InputStreamDecoder] error initializing resample context");
    }

    _audioStreams.push_back(std::shared_ptr<AudioStreamContext>(new AudioStreamContext(stream, context, resampleContext, channelLayout)));
}

bool InputStreamDecoder::HasVideoTrack() const
{
    return _videoStreamContext.stream != nullptr;
}

int InputStreamDecoder::GetNumberAudioTracks() const
{
    if (_extractAudio) {
        return _audioStreams.size();
    }
    return 0;
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
        sar = &context->sample_aspect_ratio;
    } else if (stream->sample_aspect_ratio.num) {
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

int InputStreamDecoder::GetChannelLayoutIndex(int audioTrack) const
{
    return _audioStreams[audioTrack]->channelLayout;
}

int InputStreamDecoder::GetChannels(int audioTrack) const
{
    return _audioStreams[audioTrack]->context->channels;
}

int InputStreamDecoder::GetBytesPerSample(int) const {
    return av_get_bytes_per_sample(AV_SAMPLE_FMT_S32);
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
                std::cerr << "[InputStreamDecoder] error processing frame. ErrorCode: " << processedBytes << ", gotFrame: " << gotFrame << ", type: ";
                switch (frameType) {
                    case FRAME_TYPE::AUDIO:
                        std::cerr << "AUDIO" << std::endl;
                        break;
                    case FRAME_TYPE::VIDEO:
                        std::cerr << "VIDEO" << std::endl;
                        break;
                    case FRAME_TYPE::SUBTITLES:
                        std::cerr << "SUBTITLE" << std::endl;
                        break;
                    case FRAME_TYPE::UNKNOWN:
                        std::cerr << "UNKNOWN" << std::endl;
                        break;
                } 

                // WE COULD BREAK HERE OR GO ON.. maybe I make this a user setting
                // ignore errors or something like this

                // maybe it should only be for audio ??? I dont know. We will see
                //noError = false;

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

    if (_extractAudio) {
        for (unsigned int i = 0; i < _audioStreams.size(); ++i) {
            HandleDelayedAudioFrames(i, audioCallback);
        }
    }

    if (_doneCallback) {
        _doneCallback();
    }
}

bool InputStreamDecoder::HandleFrame(AVFrame& decodedFrame, FRAME_TYPE frameType, GotVideoFrameCallbackFunction videoCallback, GotAudioFrameCallbackFunction audioCallback, int audioStreamIndex)
{
    bool success = true;

    switch (frameType) {
        case FRAME_TYPE::VIDEO:

            success = HandleVideoFrame(decodedFrame, videoCallback);

            break;
        case FRAME_TYPE::AUDIO:

            if (_extractAudio) {
                success = HandleAudioFrame(decodedFrame, audioCallback, audioStreamIndex);
            }

            break;
        case FRAME_TYPE::SUBTITLES:
        case FRAME_TYPE::UNKNOWN:
        default:
            // maybe no throwing here. time will tell...
            throw new std::runtime_error("invalid frame type");
    }

    return success;
}

bool InputStreamDecoder::HandleVideoFrame(AVFrame& decodedFrame, GotVideoFrameCallbackFunction videoCallback)
{
    bool success = true;

    AVStream *stream = _videoStreamContext.stream;
    AVPicture pic;

    avpicture_alloc(&pic, (PixelFormat)_targetVideoPixelFormat, decodedFrame.width, decodedFrame.height);
    sws_scale(_swsContext, decodedFrame.data, decodedFrame.linesize, 0, decodedFrame.height, pic.data, pic.linesize);

    RawVideoFrame *videoFrame = new RawVideoFrame();
    videoFrame->width = stream->codec->width;
    videoFrame->height = stream->codec->height;
    videoFrame->pixelFormat = stream->codec->pix_fmt;
    videoFrame->fieldOrder = stream->codec->field_order;
    videoFrame->pixelFormat = _targetVideoPixelFormat;
    videoFrame->planar = _targetVideoPixelFormat != AV_PIX_FMT_RGB24;   // PIX_FMT_24 is only packed format (here)
    videoFrame->yuv = _targetVideoPixelFormat != AV_PIX_FMT_RGB24 &&
                     _targetVideoPixelFormat != AV_PIX_FMT_GBRP10 &&
                     _targetVideoPixelFormat != AV_PIX_FMT_GBRP12;
    videoFrame->subsampled = _targetVideoPixelFormat == AV_PIX_FMT_YUV422P ||
                            _targetVideoPixelFormat == AV_PIX_FMT_YUV422P10 ||
                            _targetVideoPixelFormat == AV_PIX_FMT_YUV422P12;

    // TO-DO: Give owhership over data to _videoStreamContext (note that this is only assignment of pointers)

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < pic.linesize[i] * stream->codec->height; ++j) {
            videoFrame->videoData[i].push_back(pic.data[i][j]);
        }
        videoFrame->linesize[i] = pic.linesize[i];
    }
    // pass RawVideoFrame to user so that she can do what she wants.
    try {
        _videoStreamContext.currentFrame++;
        videoFrame->frameNumber = _videoStreamContext.currentFrame;
        success = videoCallback(videoFrame);
        avpicture_free(&pic);
    } catch (...) {
        avpicture_free(&pic);
        throw;
    }
    return success;
}

bool InputStreamDecoder::HandleAudioFrame(AVFrame& decodedFrame, GotAudioFrameCallbackFunction audioCallback, int audioStreamIndex)
{
    bool success = true;

    std::shared_ptr<AudioStreamContext> ctx = _audioStreams[audioStreamIndex];
    SwrContext *swrContext = ctx->resampleContext;

    unsigned char **out;
    int outLinesize;
    int outSamples = av_rescale_rnd(decodedFrame.nb_samples, _targetAudioSampleRate, ctx->stream->codec->sample_rate, AV_ROUND_UP);
    av_samples_alloc_array_and_samples(&out,
                                       &outLinesize,
                                       ctx->context->channels,
                                       outSamples,
                                       AV_SAMPLE_FMT_S32,
                                       0);

    int newSamples = swr_convert(swrContext,
                                  out,
                                  outSamples,
                                  (const uint8_t **)decodedFrame.extended_data,
                                  decodedFrame.nb_samples);
    if (newSamples < 0) {
        std::cerr << "[InputStreamDecoder] Error while converting [swr_convert]" << std::endl;
        av_freep(out);
        return false;
    }
    int bufferSize = av_samples_get_buffer_size(&outLinesize, ctx->context->channels, newSamples, AV_SAMPLE_FMT_S32, 1);

    RawAudioFrame frame;
    // no ownership transfer here
    frame.audioData = out[0];
    frame.linesize = bufferSize;
    frame.samples = newSamples;
    try {
        audioCallback(frame, audioStreamIndex);
    } catch (...) {
        if (out) {
            av_freep(&out[0]);
        }
        av_freep(&out);
        throw;
    }

    if (out) {
        av_freep(&out[0]);
    }
    av_freep(&out);

    return success;
}

bool InputStreamDecoder::HandleDelayedAudioFrames(int audioStreamIndex, GotAudioFrameCallbackFunction audioCallback)
{
    int newSamples = 0;
    do {
        std::shared_ptr<AudioStreamContext> ctx = _audioStreams[audioStreamIndex];
        SwrContext *swrContext = ctx->resampleContext;

        unsigned char **out;
        int outLinesize;
        int outSamples = av_rescale_rnd(1152, _targetAudioSampleRate, ctx->stream->codec->sample_rate, AV_ROUND_UP);
        av_samples_alloc_array_and_samples(&out,
                                           &outLinesize,
                                           ctx->context->channels,
                                           outSamples,
                                           AV_SAMPLE_FMT_S32,
                                           0);

        newSamples = swr_convert(swrContext,
                                 out,
                                 outSamples,
                                 nullptr,
                                 0);
        if (newSamples < 0) {
            std::cout << "[InputStreamDecoder] Error while converting - swr_convert" << std::endl;
            av_freep(out);
            return false;
        }
        int bufferSize = av_samples_get_buffer_size(&outLinesize, ctx->context->channels, newSamples, AV_SAMPLE_FMT_S32, 1);

        RawAudioFrame frame;
        // no ownership transfer here
        frame.audioData = out[0];
        frame.linesize = bufferSize;
        frame.samples = newSamples;
        try {
            audioCallback(frame, audioStreamIndex);
        } catch (...) {
            if (out) {
                av_freep(&out[0]);
            }
            av_freep(&out);
            throw;
        }

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
            //std::cout << "[VIDEO] processedBytes: " << processedBytes << std::endl;
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
            //std::cout << "[AUDIO] (" << j << ") processedBytes: " << processedBytes << std::endl;
            frameType = FRAME_TYPE::AUDIO;
            index = j;
            return processedBytes;
        }
        j++;
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
