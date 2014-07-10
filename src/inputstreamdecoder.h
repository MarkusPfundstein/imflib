#ifndef INPUTSTREAMDECODER_H
#define INPUTSTREAMDECODER_H

#include <string>
#include <memory>
#include <vector>
#include <tuple>
#include <functional>

// forward references

class Frame;
class AudioFrame;
class VideoFrame;

struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
struct AVPacket;
struct AVFrame;

struct RawVideoFrame
{
    RawVideoFrame() : bufferSize(-1), pixelFormat(0), width(0), height(0), fieldOrder(0) {}

    uint8_t *videoData[4];
    int bufferSize;

    int pixelFormat;
    int width;
    int height;

    int fieldOrder;
};

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
        typedef std::function<bool(AVFrame&)> GotAudioFrameCallbackFunction;

        static void RegisterAVFormat();

        InputStreamDecoder(const std::string& file);
        virtual ~InputStreamDecoder();

        void Decode(GotVideoFrameCallbackFunction videoCallback, GotAudioFrameCallbackFunction audioCallback);

    protected:
    private:
        void OpenFile(const std::string& file);
        void CloseFile();

        int DecodePacket(AVPacket& packet, AVFrame& decodedFrame, int &gotFrame, FRAME_TYPE &frameType);

        bool HandleFrame(AVFrame& decodedFrame, FRAME_TYPE frameType, GotVideoFrameCallbackFunction videoCallback, GotAudioFrameCallbackFunction audioCallback);

        AVFormatContext* _formatContext;

        typedef std::shared_ptr<AVCodecContext> CodecContextPtr;

        struct _VideoStreamContext {
            _VideoStreamContext()
                : stream(nullptr), context(nullptr), videoFrame() {}

            AVStream *stream;
            CodecContextPtr context;
            int videoLineSize[4];
            RawVideoFrame videoFrame;
        } _videoStreamContext;

        // we store the raw pointers to AVStream. because livetime of stream depends anyway on _formatContext
        std::vector<std::tuple<AVStream*, CodecContextPtr>> _audioStreams;
        std::vector<std::tuple<AVStream*, CodecContextPtr>> _subtitleStreams;
};

#endif // INPUTSTREAMDECODER_H
