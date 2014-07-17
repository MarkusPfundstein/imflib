#include "src/j2kencoder.h"
#include "src/inputstreamdecoder.h"
#include "src/mxfwriter.h"

#include "src/j2kframe.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <list>
#include <boost/filesystem.hpp>
#include <signal.h>

using namespace boost;

// file path for j2k files
std::string tempFilePath;

std::ofstream debugFile("/home/markus/Documents/IMF/TestFiles/DEBUG_RAW.raw", std::ios::binary | std::ios::out);

static std::string debugRawFileDirectory("/home/markus/Documents/IMF/TestFiles/RAWFILES");
static int rawCount = 0;

void WriteRawFrameToFile(const RawVideoFrame &rawFrame)
{
    debugFile.write((const char*)rawFrame.videoData[0], rawFrame.linesize[0] * rawFrame.height);
    debugFile.flush();

    std::stringstream ss;
    ss << debugRawFileDirectory << "/" << std::setw( 7 ) << std::setfill( '0' ) << rawCount << ".raw";
    std::string targetFile(ss.str());
    std::ofstream of(targetFile, std::ios::binary | std::ios::out);
    of.write((const char*)rawFrame.videoData[0], rawFrame.linesize[0] * rawFrame.height);
    of.flush();
    of.close();

    rawCount++;
}

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

    WriteRawFrameToFile(rawFrame);

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

void CleanJ2KFiles(const std::string &directory)
{
    filesystem::directory_iterator it(directory);
    filesystem::directory_iterator itEnd;
    while (it != itEnd) {
        const filesystem::path path = it->path();
        filesystem::remove(path);
        ++it;
    }
}

void SignalHandler(int sig)
{
    if (sig == SIGINT || sig == SIGQUIT || sig == SIGTERM) {
        std::cerr << "[CLEANUP]" << std::endl;
        CleanJ2KFiles(tempFilePath);
        exit(1);
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    InputStreamDecoder::RegisterAVFormat();

    bool encryptHeader = false;

    // file path to store intermediate j2k files.
    tempFilePath = "/home/markus/Documents/IMF/TestFiles/J2KFILES";
    if (!filesystem::is_directory(tempFilePath)) {
        std::cerr << tempFilePath << " is not a directory" << std::endl;
        return 1;
    }

    // FOR DEBUG
    CleanJ2KFiles(tempFilePath);

    // check if temp directory is empty. this is important.
    if (!filesystem::is_empty(tempFilePath)) {
        std::cerr << tempFilePath << " is not empty. Use -r option to empty it before encoding" << std::endl;
        return 1;
    }

    std::string inputFile = "/home/markus/Documents/IMF/TestFiles/MPEG2_PAL_SHORT.mpeg";
    //inputFile = "/home/markus/Documents/IMF/TestFiles/h264_1080p_SMALL.mp4";



    if (!filesystem::is_regular_file(inputFile)) {
        std::cerr << inputFile << " is not a file" << std::endl;
        return 1;
    }

    std::string finalVideoFile = "/home/markus/Documents/IMF/FINAL.mxf";
    if (filesystem::exists(finalVideoFile)) {
        std::cerr << finalVideoFile << " exists already. Use -f option to override" << std::endl;

        std::cout << "REMOVE FOR DEBUG MODE" << std::endl;
        filesystem::remove(filesystem::path(finalVideoFile));
        //return 1;
    }

    std::map<std::string, boost::any> muxerOptions;

    // storage for all j2k files. we need that later for asdcp lib
    std::list<std::string> j2kFiles;

    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGQUIT, SignalHandler);

    try {
        J2KEncoder::PROFILE profile = J2KEncoder::PROFILE::BCP_ST_1;

        J2KEncoder::COLOR_FORMAT colorFormat = J2KEncoder::COLOR_FORMAT::CF_RGB444;
        bool yuvEssence = false;
        if (colorFormat != J2KEncoder::COLOR_FORMAT::CF_RGB444) {
            yuvEssence = true;
        }

        // 10bit creates green video file :-)
        J2KEncoder::BIT_RATE bitsPerComponent = J2KEncoder::BIT_RATE::BR_8bit;

        std::cout << "encode with " << bitsPerComponent * 3 << " bpp" << std::endl;

        J2KEncoder j2kEncoder(colorFormat, bitsPerComponent, profile);
        InputStreamDecoder decoder(inputFile);

        decoder.Decode([&] (RawVideoFrame &rawFrame) { return HandleVideoFrame(rawFrame, j2kEncoder, j2kFiles, tempFilePath); },
                       [&] (AVFrame &) { return HandleAudioFrame(); });

        muxerOptions["framerate"] = decoder.GetFrameRate();
        muxerOptions["aspect_ratio"] = decoder.GetAspectRatio();
        muxerOptions["container_duration"] = static_cast<uint32_t>(j2kFiles.size());
        muxerOptions["yuv_essence"] = yuvEssence;
        muxerOptions["subsampling_dx"] = 1;
        muxerOptions["subsampling_dy"] = 1;
        muxerOptions["encrypt_header"] = encryptHeader;
        muxerOptions["bits"] = static_cast<int>(bitsPerComponent);
        muxerOptions["broadcast_profile"] = static_cast<int>(profile);

        MXFWriter mxfWriter(muxerOptions);
        if (j2kFiles.empty() == false) {
            mxfWriter.MuxVideoFiles(j2kFiles, finalVideoFile);
        }

        //CleanJ2KFiles(tempFilePath);
    } catch (std::runtime_error &ex) {
        std::cerr << "[EXCEPTION CAUGHT - Aborting]: " << ex.what() << std::endl;
        CleanJ2KFiles(tempFilePath);
        if (filesystem::exists(finalVideoFile)) {
            const filesystem::path path(finalVideoFile);
            filesystem::remove(path);
        }
        return 1;
    }

    return 0;
}
