#include "j2kencoder.h"
#include "inputstreamdecoder.h"
#include "mxfwriter.h"
#include "common.h"
#include "pcmencoder.h"
#include "wavmuxer.h"

#include <iostream>
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
    std::cout << "[WRITE] wrote " << encodedFrame.data.size() << " Bytes to: " << targetFile << std::endl;
}

bool HandleVideoFrame(const RawVideoFrame &rawFrame, J2KEncoder &j2kEncoder, std::list<std::string> &outFiles, const std::string& outFilePath)
{
    return true;
    std::stringstream ss;
    ss << outFilePath << "/" << std::setw( 7 ) << std::setfill( '0' ) << outFiles.size() << ".j2k";
    std::string targetFile(ss.str());

    //WriteRawFrameToFile(rawFrame);

    J2kFrame j2kFrame;
    j2kEncoder.EncodeRawFrame(rawFrame, j2kFrame);

    WriteToFile(j2kFrame, targetFile);
    outFiles.push_back(targetFile);

    std::cout << std::endl;

    return true;
}

bool HandleAudioFrame(const RawAudioFrame &rawFrame, PCMEncoder &pcmEncoder, std::vector<uint8_t> &wavData, int index)
{
    std::cout << "audio index [" << index << "]" << std::endl;

    PCMFrame pcmFrame;
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
        std::cerr << "[CLEANUP]" << std::endl;
        CleanFiles(j2kFiles);
        CleanFiles(wavFiles);
        exit(1);
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    InputStreamDecoder::RegisterAVFormat();

    struct EncoderOptions {

        EncoderOptions()
        :
        profile(J2KEncoder::PROFILE::BCP_ST_5),
        bitsPerComponent(J2KEncoder::BIT_RATE::BR_10bit),
        colorFormat(J2KEncoder::COLOR_FORMAT::CF_YUV444),
        yuvEssence(colorFormat != J2KEncoder::COLOR_FORMAT::CF_RGB444),
        useTiles(true),
        inputFile("/home/markus/Documents/IMF/TestFiles/stomp.mpeg"),
        tempFilePath("/home/markus/Documents/IMF/TestFiles/J2KFILES"),
        finalVideoFile("/home/markus/Documents/IMF/FINAL_YUV444_10bit_Profile5_MCT_4_JPEG_TRANSFORM.mxf"),
        sampleRate(PCMEncoder::SAMPLE_RATE::SR_48000),
        tempAudioFilesPath("/home/markus/Documents/IMF/TestFiles/WAVFILES")
        {
            if (useTiles && profile != J2KEncoder::PROFILE::BCP_MT_6 && profile != J2KEncoder::PROFILE::BCP_MT_7) {
                std::cout << "tried to use tiles with single tiles profile. deactive tiling" << std::endl;
                useTiles = false;
            }
        }

        /* VIDEO STUFF */
        J2KEncoder::PROFILE profile ;
        J2KEncoder::BIT_RATE bitsPerComponent;
        J2KEncoder::COLOR_FORMAT colorFormat;
        bool yuvEssence;
        bool useTiles;

        std::string inputFile;
        std::string tempFilePath;
        std::string finalVideoFile;

        /* AUDIO STUFF */
        PCMEncoder::SAMPLE_RATE sampleRate;
        std::string tempAudioFilesPath;
    };

    EncoderOptions options;


    std::cout << "encode with " << options.bitsPerComponent * 3 << " bpp" << std::endl;
    if (!filesystem::is_directory(options.tempFilePath)) {
        std::cerr << options.tempFilePath << " is not a directory" << std::endl;
        return 1;
    }

    // FOR DEBUG
    /*
    CleanDirectory(tempFilePath);

    // check if temp directory is empty. this is important.
    if (!filesystem::is_empty(tempFilePath)) {
        std::cerr << tempFilePath << " is not empty. Use -r option to empty it before encoding" << std::endl;
        return 1;
    }*/

    if (!filesystem::is_regular_file(options.inputFile)) {
        std::cerr << options.inputFile << " is not a file" << std::endl;
        return 1;
    }

    if (filesystem::exists(options.finalVideoFile)) {
        std::cerr << options.finalVideoFile << " exists already. Use -f option to override" << std::endl;

        std::cout << "REMOVE FOR DEBUG MODE" << std::endl;
        filesystem::remove(filesystem::path(options.finalVideoFile));
        //return 1;
    }

    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGQUIT, SignalHandler);

    try {

        InputStreamDecoder decoder(options.inputFile, (int)options.bitsPerComponent, (int)options.sampleRate);
        // decoder knows now some metadata about the video. Attention: IT DOESN'T KNOW ASPECT RATIO!!!!
        RationalNumber fps = decoder.GetFrameRate();


        // create one j2k encoder -> we assume only one video track
        J2KEncoder j2kEncoder(options.colorFormat, options.bitsPerComponent, options.profile, options.useTiles, fps, decoder.GetVideoWidth(), decoder.GetVideoHeight());
        if (decoder.HasVideoTrack()) {
            // j2kEncoder will throw here if video width or video height are 0. which is most likely if we push audio only
            j2kEncoder.InitEncoder();
        }

        int numberAudioTracks = decoder.GetNumberAudioTracks();

        // create one pcm encoder foreach audio track
        std::vector<std::shared_ptr<PCMEncoder>> pcmEncoders;
        // storage for all pcm data
        std::vector<std::vector<uint8_t>> wavData;
        wavData.reserve(numberAudioTracks);
        for (int i = 0; i < numberAudioTracks; ++i) {
            std::stringstream ss;
            ss << options.tempAudioFilesPath << "/AUDIO_" << (i + 1) << ".wav";
            std::string wavFile = ss.str();
            wavFiles.push_back(wavFile);

            int channelLayout = decoder.GetChannelLayoutIndex(i);
            int channels = decoder.GetChannels(i);

            std::shared_ptr<PCMEncoder> pcmEncoder(new PCMEncoder(options.sampleRate, channels, channelLayout));
            pcmEncoder->InitEncoder();
            pcmEncoders.push_back(pcmEncoder);

            wavData.push_back(std::vector<uint8_t>());
        }

        decoder.Decode([&] (RawVideoFrame &rawFrame) { return HandleVideoFrame(rawFrame, j2kEncoder, j2kFiles, options.tempFilePath); },
                       [&] (RawAudioFrame &rawFrame, int index) { return HandleAudioFrame(rawFrame, *(pcmEncoders[index]), wavData[index], index); });


        if (j2kFiles.empty() == false) {
            std::map<std::string, boost::any> muxerOptions;
            // to-do: put all this shit in a struct
            muxerOptions["framerate"] = fps;
            // Aspect Ratio is now known.
            muxerOptions["aspect_ratio"] = decoder.GetAspectRatio();
            muxerOptions["container_duration"] = static_cast<uint32_t>(j2kFiles.size());
            muxerOptions["yuv_essence"] = options.yuvEssence;
            muxerOptions["subsampling_dx"] = 1;
            muxerOptions["subsampling_dy"] = 1;
            muxerOptions["encrypt_header"] = false;
            muxerOptions["bits"] = static_cast<int>(options.bitsPerComponent);
            muxerOptions["broadcast_profile"] = static_cast<int>(options.profile);

            // write video
            MXFWriter videoMxfWriter(muxerOptions);
            videoMxfWriter.MuxVideoFiles(j2kFiles, options.finalVideoFile);
        }


        // write wav files to disk
        for (unsigned int i = 0; i < wavData.size(); ++i) {
            std::vector<uint8_t> &data = wavData[i];
            std::cout << "write pcm with " << data.size() << std::endl;

            short channels = (short)decoder.GetChannels(i);
            int sampleRate = (int)options.sampleRate;

            std::stringstream ss;
            ss << options.tempAudioFilesPath << "/" << std::setw( 7 ) << std::setfill( '0' ) << i << ".wav";

            std::string wavFileName = ss.str();

            WavMuxer wavMuxer;
            wavMuxer.MuxToFile(wavFileName, data, channels, sampleRate, 24);
        }

        // write audio
        // MXFWriter audioMxfWriter(...)
        // audioMxfWriter.MuxAudioFiles(wavFiles, ...);


        CleanFiles(j2kFiles);
        CleanFiles(wavFiles);
    } catch (std::runtime_error &ex) {
        std::cerr << "[EXCEPTION CAUGHT - Aborting]: " << ex.what() << std::endl;
        CleanFiles(j2kFiles);
        CleanFiles(wavFiles);
        if (filesystem::exists(options.finalVideoFile)) {
            const filesystem::path path(options.finalVideoFile);
            filesystem::remove(path);
        }
        return 1;
    }

    std::cout << "TO-DO LIST" << std::endl;
    std::cout << "WRITE AS WAV" << std::endl;
    std::cout << "NO RESAMPLE FLAG FOR 24bit INPUT WITH TARGET RATE" << std::endl;

    return 0;
}
