#include "src/j2kencoder.h"
#include "src/inputstreamdecoder.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

int main(int argc, char **argv)
{
    InputStreamDecoder::RegisterAVFormat();

    std::ofstream rawOutFile("/home/markus/Documents/IMF/TestFiles/OUTPUT_RAW", std::ios::binary | std::ios::out);
    if (!rawOutFile.is_open()) {
        throw std::runtime_error("couldnt open output file");
    }

    std::string inputFile = "/home/markus/Documents/IMF/TestFiles/MPEG2_PAL.mpeg";
    //inputFile = "/home/markus/Documents/IMF/TestFiles/h264_1080p.mp4";

    InputStreamDecoder decoder(inputFile);

    J2KEncoder j2kEncoder;

    decoder.Decode([&rawOutFile, &j2kEncoder] (RawVideoFrame &rawFrame) {
                    std::cout << "got raw video frame" << std::endl;

                    rawOutFile.write((const char*)rawFrame.videoData[0], rawFrame.linesize[0] * rawFrame.height);

                    J2kFrame encodedFrame;
                    return j2kEncoder.EncodeRawFrame(rawFrame, encodedFrame);
                   },
                   [] (AVFrame &rawFrame) {
                    std::cout << "got raw audio frame" << std::endl;
                    return true;
                   });

    rawOutFile.flush();
    rawOutFile.close();

    return 0;
}
