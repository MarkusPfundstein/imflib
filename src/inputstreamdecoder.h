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
struct ReSampleContext;
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
        typedef std::shared_ptr<AVCodecContext> CodecContextPtr;

        void OpenFile(const std::string& file);
        void CloseFile();
        void AddVideoStream(AVStream *stream, const CodecContextPtr &context);
        void AddAudioStream(AVStream *stream, const CodecContextPtr &context);

        int DecodePacket(AVPacket& packet, AVFrame& decodedFrame, int &gotFrame, FRAME_TYPE &frameType, int &audioStreamIndex);

        bool HandleFrame(AVFrame& decodedFrame, FRAME_TYPE frameType, GotVideoFrameCallbackFunction videoCallback, GotAudioFrameCallbackFunction audioCallback, int audioStreamIndex);

        AVFormatContext* _formatContext;



        struct VideoStreamContext {
            VideoStreamContext()
                : stream(nullptr), context(nullptr), videoFrame() {}

            VideoStreamContext(const VideoStreamContext& ) = delete;
            VideoStreamContext* operator=(const VideoStreamContext& ) = delete;

            AVStream *stream;
            CodecContextPtr context;
            RawVideoFrame videoFrame;
        } _videoStreamContext;

        struct AudioStreamContext {
            AudioStreamContext(AVStream *s, CodecContextPtr ptr, ReSampleContext *ctx)
                : stream(s), context(ptr), resampleContext(ctx) {}

            AudioStreamContext(const AudioStreamContext& o)
                : stream(o.stream), context(o.context), resampleContext(o.resampleContext) {}

            AudioStreamContext& operator=(const AudioStreamContext& o)
            {
                stream = o.stream;
                context = o.context;
                resampleContext = o.resampleContext;
                return *this;
            }

            AVStream *stream;
            CodecContextPtr context;
            ReSampleContext *resampleContext;
        };

        // we store the raw pointers to AVStream. because livetime of stream depends anyway on _formatContext
        std::vector<AudioStreamContext> _audioStreams;
        std::vector<AudioStreamContext> _subtitleStreams;

        // Context for converting color space -> should move into VideoStreamContext
        SwsContext *_swsContext;

        int _targetFormat;
};

#endif // INPUTSTREAMDECODER_H
