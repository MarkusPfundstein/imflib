#include "src/j2kencoder.h"
#include "src/inputstreamdecoder.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <iomanip>

#include <AS_02.h>

bool HandleVideoFrame(RawVideoFrame &rawFrame, J2KEncoder &j2kEncoder)
{
    static int DEBUG_WRITE_COUNT = 0;

    std::stringstream DS;
    DS << "/home/markus/Documents/IMF/TestFiles/J2KFILES/" << std::setw( 7 ) << std::setfill( '0' ) << DEBUG_WRITE_COUNT << ".j2c";
    std::string DEBUG_OUT_FILE = DS.str();

    std::cout << "got raw video frame" << std::endl;

    //rawOutFile.write((const char*)rawFrame.videoData[0], rawFrame.linesize[0] * rawFrame.height);

    J2kFrame encodedFrame;
    bool j2kSuccess = j2kEncoder.EncodeRawFrame(rawFrame, encodedFrame);
    if (j2kSuccess) {
        std::cout << "Got " << encodedFrame.data.size() << " bytes j2k data" << std::endl;

        std::ofstream of(DEBUG_OUT_FILE, std::ios::binary | std::ios::out);
        of.write((const char*)&encodedFrame.data[0], encodedFrame.data.size());


        std::cout << "GENERATED: " << DEBUG_OUT_FILE << std::endl;


        DEBUG_WRITE_COUNT++;
    }


    return j2kSuccess;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    InputStreamDecoder::RegisterAVFormat();

    J2KEncoder j2kEncoder;

    std::string inputFile = "/home/markus/Documents/IMF/TestFiles/MPEG2_PAL.mpeg";
    //inputFile = "/home/markus/Documents/IMF/TestFiles/h264_1080p.mp4";
    InputStreamDecoder decoder(inputFile);

    decoder.Decode([&j2kEncoder] (RawVideoFrame &rawFrame) { return HandleVideoFrame(rawFrame, j2kEncoder); },
                   [] (AVFrame &rawFrame) {
                    (void)rawFrame;
                    std::cout << "got raw audio frame" << std::endl;
                    return true;
                   });

    return 0;
}
