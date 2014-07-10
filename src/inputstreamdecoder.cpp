#include "inputstreamdecoder.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
//#include <libavutil/samplefmt.h>
//#include <libavutil/timestamp.h>
#ifdef __cplusplus
}
#endif

#include <iostream>
#include <mutex>

InputStreamDecoder::InputStreamDecoder(const std::string &file)
    : _formatContext(nullptr), _videoStreamContext(), _audioStreams(), _subtitleStreams()
{
    OpenFile(file);
}

InputStreamDecoder::~InputStreamDecoder()
{
    CloseFile();
}

void InputStreamDecoder::CloseFile()
{
    if (_formatContext) {
        avformat_close_input(&_formatContext);
    }
    if (_videoStreamContext.videoFrame.bufferSize > 0) {
        av_free(_videoStreamContext.videoFrame.videoData[0]);
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

    // this leaks. dont know yet how to fix it
    AVDictionary *codecOptions = nullptr;
    //av_dict_set(&codecOptions, "refcounted_frames", "1", 0);

    bool gotVideo = false;
    for (unsigned int i = 0; i < _formatContext->nb_streams; ++i) {
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

        if (avcodec_open2(codecContext.get(), codec, &codecOptions) < 0) {
            throw std::runtime_error("Could not open codec");
        }

        int bufSize = 0;

        switch (stream->codec->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                if (gotVideo) {
                    std::cout << "second video stream detected. this is not implemented yet" << std::endl;
                }
                std::cout << "video stream detected: " << stream->index << std::endl;
                //std::cout << "codec: " << stream->codec->name << std::endl;
                std::cout << "pix_fmt: " << av_get_pix_fmt_name(stream->codec->pix_fmt) << std::endl;
                std::cout << "size: " << stream->codec->width << ":" << stream->codec->height << std::endl;

                _videoStreamContext.context = codecContext;
                _videoStreamContext.stream = stream;

                // store data for later in raw video frame
                _videoStreamContext.videoFrame.width = stream->codec->width;
                _videoStreamContext.videoFrame.height = stream->codec->height;
                _videoStreamContext.videoFrame.pixelFormat = stream->codec->pix_fmt;
                _videoStreamContext.videoFrame.fieldOrder = stream->codec->field_order;

                bufSize = av_image_alloc(_videoStreamContext.videoFrame.videoData, _videoStreamContext.videoLineSize, stream->codec->width, stream->codec->height, stream->codec->pix_fmt, 1);
                if (bufSize < 0) {
                    throw std::runtime_error("Couldn't allocate image buffer");
                }

                _videoStreamContext.videoFrame.bufferSize = bufSize;

                gotVideo = true;
                break;
            case AVMEDIA_TYPE_AUDIO:
                std::cout << "audio stream detected: " << stream->index << std::endl;
                _audioStreams.push_back(std::make_tuple(stream, codecContext));
                break;
            case AVMEDIA_TYPE_SUBTITLE:
                std::cout << "subtitle stream detected: " << stream->index << std::endl;
                _subtitleStreams.push_back(std::make_tuple(stream, codecContext));
                break;
            default:
                std::cout << "invalid stream detected: " << stream->index << std::endl;
        }
    }

    if ((gotVideo == false) && _audioStreams.empty() && _subtitleStreams.empty()) {
        throw std::runtime_error("no streams detected");
    }
}

void InputStreamDecoder::Decode(GotVideoFrameCallbackFunction videoCallback, GotAudioFrameCallbackFunction audioCallback)
{
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = nullptr;

    bool noError = true;

    std::shared_ptr<AVFrame> decodedFrame(avcodec_alloc_frame(), &av_free);

    //std::ofstream outFile("/home/markus/raw", std::ofstream::out | std::ofstream::binary);
    //if (!outFile.is_open()) {
    //throw std::runtime_error("error opening raw out file");
    //}

    while (noError) {

        if (av_read_frame(_formatContext, &packet) < 0) {
            std::cout << "done" << std::endl;
            break;
        }

        // store reference to original packet. for freeing memory later on
        AVPacket origPacket = packet;

        // decode packet
        do {
            int gotFrame = 0;
            FRAME_TYPE frameType;
            int processedBytes = DecodePacket(packet, *decodedFrame, gotFrame, frameType);
            if (processedBytes < 0) {
                noError = false;
                break;
            }
            if (gotFrame) {
                bool success = HandleFrame(*decodedFrame, frameType, videoCallback, audioCallback);
                if (success == false) {
                    noError = false;
                    break;
                }
            }
            packet.data += processedBytes;
            packet.size -= processedBytes;
        } while (packet.size > 0);

        av_free_packet(&origPacket);

    }

    // flush remaining data

    packet.data = nullptr;
    packet.size = 0;

    int gotFrame = 0;
    FRAME_TYPE frameType;

    do {
        DecodePacket(packet, *decodedFrame, gotFrame, frameType);
        if (gotFrame) {
            HandleFrame(*decodedFrame, frameType, videoCallback, audioCallback);
        }
    } while (gotFrame);

    //outFile.flush();9

    //outFile.close();
}

bool InputStreamDecoder::HandleFrame(AVFrame& decodedFrame, FRAME_TYPE frameType, GotVideoFrameCallbackFunction videoCallback, GotAudioFrameCallbackFunction audioCallback)
{
    bool success = true;

    switch (frameType) {
        case FRAME_TYPE::VIDEO:
            av_image_copy(_videoStreamContext.videoFrame.videoData,
                          _videoStreamContext.videoLineSize,
                          (const uint8_t **)(decodedFrame.data),
                          decodedFrame.linesize,
                          _videoStreamContext.context->pix_fmt,
                          _videoStreamContext.context->width,
                          _videoStreamContext.context->height);

            success = videoCallback(_videoStreamContext.videoFrame);

            //outFile.write((const char*)_videoStreamContext.videoFrame.videoData[0], _videoStreamContext.videoFrame.bufferSize);
            break;
        case FRAME_TYPE::AUDIO:
            //audioCallback(*decodedFrame);
            break;
        case FRAME_TYPE::SUBTITLES:
        case FRAME_TYPE::UNKNOWN:
        default:
            break;
    }

    return success;
}

int InputStreamDecoder::DecodePacket(AVPacket& packet, AVFrame& decodedFrame, int &gotFrame, FRAME_TYPE &frameType)
{
    // search for video stream
    AVStream* stream = _videoStreamContext.stream;
    AVCodecContext *codecContext = _videoStreamContext.context.get();
    if (stream->index == packet.stream_index) {
        int processedBytes = avcodec_decode_video2(codecContext, &decodedFrame, &gotFrame, &packet);
        std::cout << "[VIDEO] processedBytes: " << processedBytes << std::endl;
        frameType = FRAME_TYPE::VIDEO;
        return processedBytes;
    }


    // search for audio stream
    for (auto i = _audioStreams.begin(); i != _audioStreams.end(); ++i) {
        AVStream* audioStream = std::get<0>(*i);
        AVCodecContext *audioCodecContext = std::get<1>(*i).get();
        if (audioStream->index == packet.stream_index) {
            int processedBytes = avcodec_decode_audio4(audioCodecContext, &decodedFrame, &gotFrame, &packet);
            std::cout << "[AUDIO] processedBytes: " << processedBytes << std::endl;
            frameType = FRAME_TYPE::AUDIO;
            return processedBytes;
        }
    }

    frameType = FRAME_TYPE::UNKNOWN;

    return -1;
}

void InputStreamDecoder::RegisterAVFormat()
{
    static std::once_flag initFlag;
    std::call_once(initFlag, []() { av_register_all(); });
}
