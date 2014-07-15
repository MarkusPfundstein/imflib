#include "src/j2kencoder.h"
#include "src/inputstreamdecoder.h"
#include "src/mxfwriter.h"

#include "src/j2kframe.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <vector>
#include <boost/filesystem.hpp>
#include <signal.h>

using namespace boost;

// file path for j2k files
std::string tempFilePath;

void WriteToFile(const J2kFrame &encodedFrame, const std::string &targetFile)
{
    std::ofstream of(targetFile, std::ios::binary | std::ios::out);
    of.write((const char*)&encodedFrame.data[0], encodedFrame.data.size());
    std::cout << "[WRITE] wrote " << encodedFrame.data.size() << " Bytes to: " << targetFile << std::endl;
}

bool HandleVideoFrame(RawVideoFrame &rawFrame, J2KEncoder &j2kEncoder, std::vector<std::string> &outFiles, const std::string& outFilePath)
{
    std::stringstream ss;
    ss << outFilePath << "/" << std::setw( 7 ) << std::setfill( '0' ) << outFiles.size() << ".j2k";
    std::string targetFile(ss.str());

    //write raw file...
    //rawOutFile.write((const char*)rawFrame.videoData[0], rawFrame.linesize[0] * rawFrame.height);

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

    std::string inputFile = "/home/markus/Documents/IMF/TestFiles/MPEG2_PAL.mpeg";
    inputFile = "/home/markus/Documents/IMF/TestFiles/h264_1080p_SMALL.mp4";

    if (!filesystem::is_regular_file(inputFile)) {
        std::cerr << inputFile << " is not a file" << std::endl;
        return 1;
    }

    std::string finalVideoFile = "/home/markus/Documents/IMF/FINAL.mxf";
    if (filesystem::exists(finalVideoFile)) {
        std::cerr << finalVideoFile << " exists already. Use -f option to override" << std::endl;
        return 1;
    }

    std::map<std::string, boost::any> muxerOptions;

    // storage for all j2k files. we need that later for asdcp lib
    std::vector<std::string> j2kFiles;

    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGQUIT, SignalHandler);

    try {
        J2KEncoder::COLOR_FORMAT cf = J2KEncoder::COLOR_FORMAT::CF_RGB444;
        bool yuvEssence = false;
        if (cf != J2KEncoder::COLOR_FORMAT::CF_RGB444) {
            yuvEssence = true;
        }

        J2KEncoder j2kEncoder(J2KEncoder::COLOR_FORMAT::CF_RGB444, J2KEncoder::BIT_RATE::BR_8bit);
        InputStreamDecoder decoder(inputFile);

        decoder.Decode([&] (RawVideoFrame &rawFrame) { return HandleVideoFrame(rawFrame, j2kEncoder, j2kFiles, tempFilePath); },
                       [&] (AVFrame &) { return HandleAudioFrame(); });

        muxerOptions["framerate"] = decoder.GetFrameRate();
        muxerOptions["aspect_ratio"] = decoder.GetAspectRatio();
        muxerOptions["container_duration"] = static_cast<uint32_t>(j2kFiles.size());
        muxerOptions["yuv_essence"] = yuvEssence;

        MXFWriter mxfWriter(muxerOptions);
        if (j2kFiles.empty() == false) {
            mxfWriter.MuxVideoFiles(tempFilePath, finalVideoFile);
        }

        CleanJ2KFiles(tempFilePath);
    } catch (std::runtime_error &ex) {
        std::cerr << "error decoding: " << ex.what() << std::endl;
        CleanJ2KFiles(tempFilePath);
        return 1;
    }

    return 0;
}
