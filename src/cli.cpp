#include "j2kencoder.h"
#include "inputstreamdecoder.h"
#include "mxfwriter.h"
#include "common.h"
#include "pcmencoder.h"
#include "wavmuxer.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <list>
#include <boost/filesystem.hpp>
#include <signal.h>

using namespace boost;

// needs to be global for signal handler
// storage for all j2k files. we need that later for asdcp lib
std::list<std::string> j2kFiles;
// storage for all wav files
std::list<std::string> wavFiles;

struct EncoderOptions {

    EncoderOptions()
    :
    overwriteFiles(true),
    editRate(0, 0),
    profile(J2KEncoder::PROFILE::BCP_ST_5),
    bitsPerComponent(J2KEncoder::BIT_RATE::BR_10bit),
    colorFormat(COLOR_FORMAT::CF_YUV444),
    useTiles(true),
    inputFile("/home/markus/Documents/IMF/TestFiles/stomp.wav"),
    tempFilePath("/home/markus/Documents/IMF/TestFiles/TEMP"),
    outputPath("/home/markus/Documents/IMF/TestFiles/OUTPUT"),
    sampleRate(PCMEncoder::SAMPLE_RATE::SR_48000)
    {
        if (useTiles && profile != J2KEncoder::PROFILE::BCP_MT_6 && profile != J2KEncoder::PROFILE::BCP_MT_7) {
            std::cout << "tried to use tiles with single tiles profile. deactive tiling" << std::endl;
            useTiles = false;
        }
    }

    /* cmd line stuff */
    bool overwriteFiles;
    RationalNumber editRate;

    /* VIDEO STUFF */
    J2KEncoder::PROFILE profile ;
    J2KEncoder::BIT_RATE bitsPerComponent;
    COLOR_FORMAT colorFormat;
    bool useTiles;

    std::string inputFile;
    std::string tempFilePath;

    std::string outputPath;

    /* AUDIO STUFF */
    PCMEncoder::SAMPLE_RATE sampleRate;
};

/*
void WriteRawFrameToFile(const RawVideoFrame &rawFrame)
{

    static std::ofstream debugFile("/home/markus/Documents/IMF/TestFiles/DEBUG_RAW.raw", std::ios::binary | std::ios::out);
    static std::string debugRawFileDirectory("/home/markus/Documents/IMF/TestFiles/RAWFILES");
    static int rawCount = 0;
    // write a raw video file
    debugFile.write((const char*)rawFrame.videoData[0], rawFrame.linesize[0] * rawFrame.height);
    debugFile.flush();

    // write frame in separate raw image
    std::stringstream ss;
    ss << debugRawFileDirectory << "/" << std::setw( 7 ) << std::setfill( '0' ) << rawCount << ".raw";
    std::string targetFile(ss.str());
    std::ofstream of(targetFile, std::ios::binary | std::ios::out);
    of.write((const char*)rawFrame.videoData[0], rawFrame.linesize[0] * rawFrame.height);
    of.flush();
    of.close();

    rawCount++;
}
*/

void WriteToFile(const J2kFrame &encodedFrame, const std::string &targetFile)
{
    std::ofstream of(targetFile, std::ios::binary | std::ios::out);
    of.write((const char*)&encodedFrame.data[0], encodedFrame.data.size());
}

bool HandleVideoFrame(const RawVideoFrame &rawFrame, J2KEncoder &j2kEncoder, std::list<std::string> &outFiles, const std::string& outFilePath)
{
    std::stringstream ss;
    ss << outFilePath << "/" << std::setw( 7 ) << std::setfill( '0' ) << outFiles.size() << ".j2k";
    std::string targetFile(ss.str());

    J2kFrame j2kFrame;
    j2kEncoder.EncodeRawFrame(rawFrame, j2kFrame);

    WriteToFile(j2kFrame, targetFile);
    outFiles.push_back(targetFile);

    std::cout << "Frame: " << outFiles.size() << '\xd';
    std::cout.flush();

    return true;
}

bool HandleAudioFrame(const RawAudioFrame &rawFrame, PCMEncoder &pcmEncoder, std::vector<uint8_t> &wavData, int index)
{
    (void)index;
    pcmEncoder.EncodeRawFrame(rawFrame, wavData);
    return true;
}

void CleanDirectory(const std::string &directory)
{
    filesystem::directory_iterator it(directory);
    filesystem::directory_iterator itEnd;
    while (it != itEnd) {
        const filesystem::path path = it->path();
        filesystem::remove(path);
        ++it;
    }
}

void CleanFiles(const std::list<std::string>& files)
{
    for (const std::string &s : files) {
        const filesystem::path path = s;
        filesystem::remove(path);
    }
}

void SignalHandler(int sig)
{
    if (sig == SIGINT || sig == SIGQUIT || sig == SIGTERM) {
        CleanFiles(j2kFiles);
        CleanFiles(wavFiles);
        exit(1);
    }
}

std::string GetAudioFileName(const EncoderOptions &options, int channels, int bitsPerSample, int index)
{
    std::stringstream ss;

    ss << options.outputPath << "/IMF_ODM_PCM_" << (int)options.sampleRate << "_" << channels << "ch" << "_" << bitsPerSample << "bits_" << index << ".mxf";
    return ss.str();
}

std::string GetVideoFileName(const EncoderOptions &options, int width, int height)
{
    std::stringstream ss;

    ss << options.outputPath << "/IMF_ODM_JPEG2000_" << width << "x" << height << "_";
    switch (options.colorFormat) {
        case COLOR_FORMAT::CF_RGB444:
            ss << "RGB444";
            break;
        case COLOR_FORMAT::CF_YUV444:
            ss << "YUV444";
            break;
        case COLOR_FORMAT::CF_YUV422:
            ss << "YUV422";
            break;
        default:
            throw new std::runtime_error("unknown colorformat");
    }

    std::stringstream fpsStream;
    if (options.editRate.denum != 1) {
        fpsStream << std::fixed << std::setprecision(2) << (float)options.editRate.num / options.editRate.denum;
    } else {
        fpsStream << (float)options.editRate.num;
    }
    std::string fpsString = fpsStream.str();

    // replace any dot that could occur in framerate calculations. for instance 23.97
    fpsString.erase(std::remove_if(std::begin(fpsString), std::end(fpsString), [](char c) { return (c == '.');}),
                    std::end(fpsString));

    ss << "_BCPL" << (int) options.profile << "_" << fpsString << "fps_" << options.bitsPerComponent << "bits.mxf";

    return ss.str();
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    InputStreamDecoder::RegisterAVFormat();

    EncoderOptions options;

    std::cout << "encode with " << options.bitsPerComponent * 3 << " bpp" << std::endl;
    if (!filesystem::is_directory(options.tempFilePath)) {
        std::cerr << options.tempFilePath << " is not a directory" << std::endl;
        return 1;
    }

    if (!filesystem::is_directory(options.outputPath)) {
        std::cerr << options.outputPath << " is not a directory" << std::endl;
        return 1;
    }

    if (!filesystem::is_regular_file(options.inputFile)) {
        std::cerr << options.inputFile << " is not a file" << std::endl;
        return 1;
    }

    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGQUIT, SignalHandler);

    try {

        InputStreamDecoder decoder(options.inputFile, (int)options.bitsPerComponent, options.colorFormat, (int)options.sampleRate);
        // decoder knows now some metadata about the video. Attention: IT DOESN'T KNOW ASPECT RATIO!!!!


        // get filename for video file.
        std::string finalVideoFile;

        // create one j2k encoder -> we assume only one video track
        J2KEncoder j2kEncoder(options.bitsPerComponent, options.profile, options.useTiles, options.editRate, decoder.GetVideoWidth(), decoder.GetVideoHeight());
        if (decoder.HasVideoTrack()) {
            finalVideoFile = GetVideoFileName(options, decoder.GetVideoWidth(), decoder.GetVideoHeight());

            options.editRate = decoder.GetFrameRate();

            // TO-DO: Add check if user really wants to delete file. otherwise abort if exists
            if (filesystem::exists(finalVideoFile)) {
                if (options.overwriteFiles) {
                    const filesystem::path path(finalVideoFile);
                    filesystem::remove(path);
                } else {
                    std::cerr << finalVideoFile << " exists already. Run with -f to overwrite!" << std::endl;
                    return 1;
                }
            }
            // j2kEncoder will throw here if video width or video height are 0. which is most likely if we push audio only
            j2kEncoder.InitEncoder();
        }

        int numberAudioTracks = decoder.GetNumberAudioTracks();
        if (numberAudioTracks > 0 && decoder.HasVideoTrack() == false && options.editRate.num == 0) {
            std::cerr << "No Video Track found. Please set edit rate for audio files with -r option" << std::endl;
            return 1;
        }

        // create one pcm encoder foreach audio track
        std::vector<std::shared_ptr<PCMEncoder>> pcmEncoders;
        // storage for all pcm data
        std::vector<std::vector<uint8_t>> wavData;
        wavData.reserve(numberAudioTracks);
        for (int i = 0; i < numberAudioTracks; ++i) {
            int channelLayout = decoder.GetChannelLayoutIndex(i);
            int channels = decoder.GetChannels(i);

            std::shared_ptr<PCMEncoder> pcmEncoder(new PCMEncoder(options.sampleRate, channels, channelLayout));
            pcmEncoder->InitEncoder();
            pcmEncoders.push_back(pcmEncoder);

            wavData.push_back(std::vector<uint8_t>());
        }

        decoder.Decode([&] (RawVideoFrame &rawFrame) { return HandleVideoFrame(rawFrame, j2kEncoder, j2kFiles, options.tempFilePath); },
                       [&] (RawAudioFrame &rawFrame, int index) { return HandleAudioFrame(rawFrame, *(pcmEncoders[index]), wavData[index], index); });


        if (decoder.HasVideoTrack() && j2kFiles.empty() == false) {
            std::map<std::string, boost::any> muxerOptions;
            // to-do: put all this shit in a struct
            muxerOptions["framerate"] = options.editRate;
            // Aspect Ratio is now known.
            muxerOptions["aspect_ratio"] = decoder.GetAspectRatio();
            muxerOptions["container_duration"] = static_cast<uint32_t>(j2kFiles.size());
            muxerOptions["yuv_essence"] = options.colorFormat != COLOR_FORMAT::CF_RGB444;
            muxerOptions["subsampling_dx"] = 1;
            muxerOptions["subsampling_dy"] = 1;
            muxerOptions["encrypt_header"] = false;
            muxerOptions["bits"] = static_cast<int>(options.bitsPerComponent);
            muxerOptions["broadcast_profile"] = static_cast<int>(options.profile);

            // write video
            MXFWriter videoMxfWriter(muxerOptions);
            videoMxfWriter.MuxVideoFiles(j2kFiles, finalVideoFile);
        }

        // write wav files to disk
        for (unsigned int i = 0; i < wavData.size(); ++i) {
            std::vector<uint8_t> &data = wavData[i];

            short channels = (short)decoder.GetChannels(i);
            int sampleRate = (int)options.sampleRate;

            std::stringstream ss;
            ss << options.tempFilePath << "/AUDIO_" << std::setw( 7 ) << std::setfill( '0' ) << i << ".wav";

            std::string wavFileName = ss.str();

            WavMuxer wavMuxer;
            wavMuxer.MuxToFile(wavFileName, data, channels, sampleRate, 24);

            wavFiles.push_back(wavFileName);

            std::map<std::string, boost::any> muxerOptions;
            muxerOptions["framerate"] = options.editRate;

            std::string finalFile = GetAudioFileName(options, channels, 24, i);
            MXFWriter audioMxfWriter(muxerOptions);
            audioMxfWriter.MuxAudioFile(wavFileName, finalFile);
        }

        CleanFiles(j2kFiles);
        CleanFiles(wavFiles);
    } catch (std::runtime_error &ex) {
        std::cerr << "[EXCEPTION CAUGHT - Aborting]: " << ex.what() << std::endl;
        CleanFiles(j2kFiles);
        CleanFiles(wavFiles);
        return 1;
    }

    return 0;
}
