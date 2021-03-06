#ifndef INPUTSTREAMDECODER_H
#define INPUTSTREAMDECODER_H

#include <string>
#include <memory>
#include <vector>
#include <tuple>
#include <functional>

#include "common.h"

extern "C" {
#include <libavformat/avformat.h>
}

// forward references

class Frame;
class AudioFrame;
class VideoFrame;

struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
struct AVPacket;
struct AVFrame;
struct SwrContext;
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

        typedef std::function<bool(RawVideoFrame*)> GotVideoFrameCallbackFunction;
        typedef std::function<bool(RawAudioFrame&, int)> GotAudioFrameCallbackFunction;

        static void RegisterAVFormat();

        explicit InputStreamDecoder(const std::string& file, int depth, COLOR_FORMAT targetColorFormat, int audioRate, bool extractAudio);

        InputStreamDecoder(const InputStreamDecoder& ) = delete;
        InputStreamDecoder* operator=(const InputStreamDecoder& ) = delete;

        virtual ~InputStreamDecoder();

        void Decode(GotVideoFrameCallbackFunction videoCallback, GotAudioFrameCallbackFunction audioCallback);

        RationalNumber GetFrameRate() const;
        RationalNumber GetAspectRatio() const;
        int GetVideoWidth() const ;
        int GetVideoHeight() const;

        int GetNumberAudioTracks() const;
        int GetChannelLayoutIndex(int audioTrack) const;
        int GetChannels(int audioTrack) const;
        int GetBytesPerSample(int audioTrack) const;

        bool HasVideoTrack() const;

        void SetDoneCallback(std::function<void(void)> doneCallback)
        { _doneCallback = doneCallback; }

    protected:
    private:
        typedef std::shared_ptr<AVCodecContext> CodecContextPtr;

        void OpenFile(const std::string& file);
        void CloseFile();
        void AddVideoStream(AVStream *stream, const CodecContextPtr &context);
        void AddAudioStream(AVStream *stream, const CodecContextPtr &context);

        bool HandleDelayedAudioFrames(int audioStreamIndex, GotAudioFrameCallbackFunction audioCallback);

        int DecodePacket(AVPacket& packet, AVFrame& decodedFrame, int &gotFrame, FRAME_TYPE &frameType, int &audioStreamIndex);

        bool HandleFrame(AVFrame& decodedFrame, FRAME_TYPE frameType, GotVideoFrameCallbackFunction videoCallback, GotAudioFrameCallbackFunction audioCallback, int audioStreamIndex);

        bool HandleVideoFrame(AVFrame& decodedFrame, GotVideoFrameCallbackFunction videoCallback);
        bool HandleAudioFrame(AVFrame& decodedFrame, GotAudioFrameCallbackFunction audioCallback, int audioStreamIndex);

        AVFormatContext* _formatContext;

        struct VideoStreamContext {
            VideoStreamContext()
                : stream(nullptr), context(nullptr), currentFrame(0) {}

            VideoStreamContext(const VideoStreamContext& ) = delete;
            VideoStreamContext* operator=(const VideoStreamContext& ) = delete;

            AVStream *stream;
            CodecContextPtr context;
            int currentFrame;
        } _videoStreamContext;

        struct AudioStreamContext {
            AudioStreamContext(AVStream *s, CodecContextPtr ptr, SwrContext *ctx, int layout)
                : stream(s), context(ptr), resampleContext(ctx), channelLayout(layout) {}

            AudioStreamContext(const AudioStreamContext& ) = delete;
            AudioStreamContext& operator=(const AudioStreamContext& ) = delete;

            AVStream *stream;
            CodecContextPtr context;
            SwrContext *resampleContext;

            int channelLayout;
        };

        // we store the raw pointers to AVStream. because livetime of stream depends anyway on _formatContext
        std::vector<std::shared_ptr<AudioStreamContext>> _audioStreams;
        std::vector<AudioStreamContext> _subtitleStreams;

        // Context for converting color space -> should move into VideoStreamContext
        SwsContext *_swsContext;

        int _targetVideoPixelFormat;

        int _targetAudioSampleRate;

        std::function<void(void)> _doneCallback;

        bool _extractAudio;
};

#endif // INPUTSTREAMDECODER_H
