#ifndef INPUTSTREAMDECODER_H
#define INPUTSTREAMDECODER_H

#include <string>
#include <memory>
#include <vector>
#include <tuple>
#include <functional>

#include "common.h"


// forward references

class Frame;
class AudioFrame;
class VideoFrame;

struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
struct AVPacket;
struct AVFrame;

struct SwsContext;

class InputStreamDecoder
{
    public:
        enum FRAME_TYPE {
            UNKNOWN = -1,
            VIDEO = 0,
            AUDIO = 1,
            SUBTITLES = 2
        };

        typedef std::function<bool(RawVideoFrame&)> GotVideoFrameCallbackFunction;
        typedef std::function<bool(AVFrame&, int)> GotAudioFrameCallbackFunction;

        static void RegisterAVFormat();

        explicit InputStreamDecoder(const std::string& file, int depth, bool yuv);

        InputStreamDecoder(const InputStreamDecoder& ) = delete;
        InputStreamDecoder* operator=(const InputStreamDecoder& ) = delete;

        virtual ~InputStreamDecoder();

        void Decode(GotVideoFrameCallbackFunction videoCallback, GotAudioFrameCallbackFunction audioCallback);

        RationalNumber GetFrameRate() const;
        RationalNumber GetAspectRatio() const;
        int GetVideoWidth() const ;
        int GetVideoHeight() const;

        int GetNumberAudioTracks() const;

        bool HasVideoTrack() const;

    protected:
    private:
        void OpenFile(const std::string& file);
        void CloseFile();

        int DecodePacket(AVPacket& packet, AVFrame& decodedFrame, int &gotFrame, FRAME_TYPE &frameType, int &audioStreamIndex);

        bool HandleFrame(AVFrame& decodedFrame, FRAME_TYPE frameType, GotVideoFrameCallbackFunction videoCallback, GotAudioFrameCallbackFunction audioCallback, int audioStreamIndex);

        AVFormatContext* _formatContext;

        typedef std::shared_ptr<AVCodecContext> CodecContextPtr;

        struct VideoStreamContext {
            VideoStreamContext()
                : stream(nullptr), context(nullptr), videoFrame() {}

            VideoStreamContext(const VideoStreamContext& ) = delete;
            VideoStreamContext* operator=(const VideoStreamContext& ) = delete;

            AVStream *stream;
            CodecContextPtr context;
            int videoLineSize[4];
            RawVideoFrame videoFrame;
        } _videoStreamContext;

        // we store the raw pointers to AVStream. because livetime of stream depends anyway on _formatContext
        std::vector<std::tuple<AVStream*, CodecContextPtr>> _audioStreams;
        std::vector<std::tuple<AVStream*, CodecContextPtr>> _subtitleStreams;

        // Context for converting color space
        SwsContext *_swsContext;

        int _targetFormat;
};

#endif // INPUTSTREAMDECODER_H
