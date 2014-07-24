#include "j2kencoder.h"
#include "inputstreamdecoder.h"
#include "mxfwriter.h"
#include "common.h"

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
    std::stringstream ss;
    ss << outFilePath << "/" << std::setw( 7 ) << std::setfill( '0' ) << outFiles.size() << ".j2k";
    std::string targetFile(ss.str());

    //WriteRawFrameToFile(rawFrame);

    J2kFrame encodedFrame;
    j2kEncoder.EncodeRawFrame(rawFrame, encodedFrame);

    WriteToFile(encodedFrame, targetFile);
    outFiles.push_back(targetFile);

    std::cout << std::endl;

    return true;
}

bool HandleAudioFrame()
{
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

void CleanJ2KFiles(const std::list<std::string>& files)
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
        CleanJ2KFiles(j2kFiles);
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
        profile(J2KEncoder::PROFILE::BCP_ST_4),
        bitsPerComponent(J2KEncoder::BIT_RATE::BR_8bit),
        colorFormat(J2KEncoder::COLOR_FORMAT::CF_RGB444),
        yuvEssence(colorFormat != J2KEncoder::COLOR_FORMAT::CF_RGB444),
        useTiles(true),
        inputFile("/home/markus/Documents/IMF/TestFiles/MPEG2_PAL_SHORT.mpeg"),
        tempFilePath("/home/markus/Documents/IMF/TestFiles/J2KFILES"),
        finalVideoFile("/home/markus/Documents/IMF/FINAL.mxf")
        {
            if (useTiles && profile != J2KEncoder::PROFILE::BCP_MT_6 && profile != J2KEncoder::PROFILE::BCP_MT_7) {
                std::cout << "tried to use tiles with single tiles profile. deactive tiling" << std::endl;
                useTiles = false;
            }
        }


        J2KEncoder::PROFILE profile ;
        J2KEncoder::BIT_RATE bitsPerComponent;
        J2KEncoder::COLOR_FORMAT colorFormat;
        bool yuvEssence;
        bool useTiles;

        std::string inputFile;
        std::string tempFilePath;
        std::string finalVideoFile;
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

    std::map<std::string, boost::any> muxerOptions;

    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGQUIT, SignalHandler);

    try {

        InputStreamDecoder decoder(options.inputFile, static_cast<int>(options.bitsPerComponent), options.yuvEssence);

        RationalNumber fps = decoder.GetFrameRate();

        J2KEncoder j2kEncoder(options.colorFormat, options.bitsPerComponent, options.profile, options.useTiles, fps, decoder.GetVideoWidth(), decoder.GetVideoHeight());
        j2kEncoder.InitEncoder();

        decoder.Decode([&] (RawVideoFrame &rawFrame) { return HandleVideoFrame(rawFrame, j2kEncoder, j2kFiles, options.tempFilePath); },
                       [&] (AVFrame &) { return HandleAudioFrame(); });

        // to-do: put all this shit in a struct
        muxerOptions["framerate"] = fps;
        // this one cant be set before decoding. on some codecs it would return wrong value
        muxerOptions["aspect_ratio"] = decoder.GetAspectRatio();
        muxerOptions["container_duration"] = static_cast<uint32_t>(j2kFiles.size());
        muxerOptions["yuv_essence"] = options.yuvEssence;
        muxerOptions["subsampling_dx"] = 1;
        muxerOptions["subsampling_dy"] = 1;
        muxerOptions["encrypt_header"] = false;
        muxerOptions["bits"] = static_cast<int>(options.bitsPerComponent);
        muxerOptions["broadcast_profile"] = static_cast<int>(options.profile);

        MXFWriter mxfWriter(muxerOptions);
        if (j2kFiles.empty() == false) {
            mxfWriter.MuxVideoFiles(j2kFiles, options.finalVideoFile);
        }

        CleanJ2KFiles(j2kFiles);
    } catch (std::runtime_error &ex) {
        std::cerr << "[EXCEPTION CAUGHT - Aborting]: " << ex.what() << std::endl;
        CleanJ2KFiles(j2kFiles);
        if (filesystem::exists(options.finalVideoFile)) {
            const filesystem::path path(options.finalVideoFile);
            filesystem::remove(path);
        }
        return 1;
    }

    return 0;
}
