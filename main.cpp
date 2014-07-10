#include "src/j2kencoder.h"
#include "src/inputstreamdecoder.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

int main(int argc, char **argv)
{
    InputStreamDecoder::RegisterAVFormat();

    std::ofstream outFile("/home/markus/Documents/IMF/TestFiles/OUTPUT_RAW", std::ios::binary | std::ios::out);
    if (!outFile.is_open()) {
        throw std::runtime_error("couldnt open output file");
    }

    std::string inputFile = "/home/markus/Documents/IMF/TestFiles/MPEG2_PAL.mpeg";
    // std::string inputFile = ""/home/markus/Documents/IMF/TestFiles/h264_1080p.mp4";

    InputStreamDecoder decoder(inputFile);

    J2KEncoder j2kEncoder;

    decoder.Decode([&outFile] (RawVideoFrame &frame) {
                    std::cout << "got raw video frame" << std::endl;

                    outFile.write((const char*)frame.videoData[0], frame.bufferSize);

                    //j2kEncoder.encode(frame, [&outFile] (JPEG2KFrame& jpegFrame) {
                    //                  });

                    return true;
                   },
                   [] (AVFrame &frame) {
                    std::cout << "got raw audio frame" << std::endl;
                    return true;
                   });

    outFile.flush();

    return 0;
}
