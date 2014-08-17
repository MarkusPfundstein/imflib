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

#include <signal.h>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

using namespace boost;

// needs to be global for signal handler
// storage for all j2k files. we need that later for asdcp lib
std::list<std::string> j2kFiles;
// storage for all wav files
std::list<std::string> wavFiles;

typedef struct {
    /* cmd line stuff */
    bool overwriteFiles;
    RationalNumber editRate;

    /* VIDEO STUFF */
    J2KEncoder::PROFILE profile ;
    J2KEncoder::BIT_RATE bitsPerComponent;
    COLOR_FORMAT colorFormat;
    bool useTiles;
    bool fullRange;
    bool doMct;

    std::string inputFile;
    std::string tempFilePath;

    std::string outputPath;

    /* AUDIO STUFF */
    PCMEncoder::SAMPLE_RATE sampleRate;
} EncoderOptions;

bool ParseProgramOptions(EncoderOptions& options, int argc, char **argv)
{
    using namespace boost::program_options;

    int profile;
    bool fullRange;
    bool forceOverwrite = false;
    bool useTiles = false;
    int bitDepth;
    int sampleRate;
    std::string colorFormat;
    std::string inputFile;
    std::string outDirectory;
    std::string tempDirectory;

    options_description description("Allowed Options");
    description.add_options()
        ("help,h", "show help")
        ("in,i", value<std::string>(&inputFile), "input file")
        ("out,o", value<std::string>(&outDirectory), "directory where output files shall be stored")
        ("temp_dir,t", value<std::string>(&tempDirectory), "directory for temporary files, defaults to output directory")
        ("force,f", "overwrite output files")
        ("broadcast_profile,b", value<int>(&profile)->default_value(1), "broadcast profile (1-7)")
        ("bitdepth,d", value<int>(&bitDepth)->default_value(10), "target video bit depth (8, 10 or 12)")
        ("sample_rate,r", value<int>(&sampleRate)->default_value(48000), "target audio samplerate (48000 or 96000)")
        ("pixel_fmt,p", value<std::string>(&colorFormat)->default_value("YUV444"), "pixel format of output. (YUV444, YUV422, RGB444)")
        ("use_tiles", "use tiles (only broadcast profile 6 and 7) [default: false]")
        ("full_range", value<bool>(&fullRange)->default_value(true), "full range color space (rgb essence only), else SMPTE 274M-2008 constraints are used");

    variables_map vm;
    store(parse_command_line(argc, argv, description), vm);
    notify(vm);

    if (vm.count("help")) {
        std::cout << description << std::endl;
        return false;
    }

    if (vm.count("force")) {
        forceOverwrite = true;
    }

    if (vm.count("in") == 0) {
        std::cerr << "no input file specified (-i)" << std::endl;
        return false;
    }

    if (vm.count("out") == 0) {
        std::cerr << "no output directory specified (-o)" << std::endl;
        return false;
    }

    if (vm.count("temp_dir") == 0) {
        tempDirectory = outDirectory;
    }

    if (vm.count("use_tiles")) {
        useTiles = true;
    }

    if (profile < 1 || profile > 7) {
        std::cerr << "profile must be between 1 and 7" << std::endl;
        return false;
    }
    if (bitDepth != 8 && bitDepth != 10 && bitDepth != 12) {
        std::cerr << "bitdepth must be 8, 10 or 12" << std::endl;
        return false;
    }
    if (sampleRate != 48000 && sampleRate != 96000) {
        std::cerr << "sample rate must be 48000 or 96000" << std::endl;
        return false;
    }
    if (colorFormat == "RGB444") {
        options.colorFormat = COLOR_FORMAT::CF_RGB444;
        options.doMct = true;
    } else if (colorFormat == "YUV444") {
        options.colorFormat = COLOR_FORMAT::CF_YUV444;
        options.doMct = false;
    } else if (colorFormat == "YUV422") {
        options.colorFormat = COLOR_FORMAT::CF_YUV422;
        options.doMct = false;
    } else {
        std::cerr << "invalid color format. Must be RGB444, YUV444 or YUV422" << std::endl;
        return false;
    }

    options.profile = (J2KEncoder::PROFILE) profile;
    options.fullRange = fullRange;
    options.overwriteFiles = forceOverwrite;
    options.useTiles = useTiles;
    options.bitsPerComponent = (J2KEncoder::BIT_RATE) bitDepth;
    options.sampleRate = (PCMEncoder::SAMPLE_RATE)sampleRate;
    options.inputFile = inputFile;
    options.tempFilePath = tempDirectory;
    options.outputPath = outDirectory;

    // fixed options (for now)
    options.editRate = RationalNumber(0, 0);

    // sanity checks
    if (!filesystem::is_directory(options.tempFilePath)) {
        std::cerr << options.tempFilePath << " is not a directory or doesn't exist" << std::endl;
        return false;
    }

    if (!filesystem::is_directory(options.outputPath)) {
        std::cerr << options.outputPath << " is not a directory or doesn't exist" << std::endl;
        return false;
    }

    if (!filesystem::is_regular_file(options.inputFile)) {
        std::cerr << options.inputFile << " is not a file or doesn't exist" << std::endl;
        return false;
    }

    if (options.useTiles && options.profile != J2KEncoder::PROFILE::BCP_MT_6 && options.profile != J2KEncoder::PROFILE::BCP_MT_7) {
        std::cout << "Tried to use tiles with single tiles profile." << std::endl;
        return false;
    }

    if (options.doMct && options.colorFormat != COLOR_FORMAT::CF_RGB444) {
        std::cout << "Tried to do mct on non RGB essence." << std::endl;
        return false;
    }

    return true;
}

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
            ss << "RGB444_" << (options.fullRange ? "RngFull" : "Rng274M");
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
    if (ParseProgramOptions(options, argc, argv) == false) {
        return 1;
    }

    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGQUIT, SignalHandler);

    try {

        InputStreamDecoder decoder(options.inputFile, (int)options.bitsPerComponent, options.colorFormat, (int)options.sampleRate);
        // decoder knows now some metadata about the video. Attention: IT DOESN'T KNOW ASPECT RATIO AT THIS PLACE!!!!

        std::string finalVideoFile;

        // create one j2k encoder -> we assume only one video track

        options.editRate = decoder.GetFrameRate();

        J2KEncoder j2kEncoder(options.bitsPerComponent,
                              options.profile,
                              options.useTiles,
                              options.editRate,
                              decoder.GetVideoWidth(),
                              decoder.GetVideoHeight(),
                              options.doMct);

        if (decoder.HasVideoTrack()) {
            finalVideoFile = GetVideoFileName(options, decoder.GetVideoWidth(), decoder.GetVideoHeight());

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
        // PROBABLY NOT NECESSARY. Edit_rate is SampleRate / 1
        //if (numberAudioTracks > 0 && decoder.HasVideoTrack() == false && options.editRate.num == 0) {
            //std::cerr << "No Video Track found. Please set edit rate for audio files with -r option" << std::endl;
            //return 1;
        //}

        // create one pcm encoder foreach audio track
        std::vector<std::shared_ptr<PCMEncoder>> pcmEncoders;
        // storage for all pcm data
        std::vector<std::vector<uint8_t>> wavData;
        std::vector<std::string> audioFiles;
        wavData.reserve(numberAudioTracks);
        for (int i = 0; i < numberAudioTracks; ++i) {
            int channelLayout = decoder.GetChannelLayoutIndex(i);
            int channels = decoder.GetChannels(i);

            std::shared_ptr<PCMEncoder> pcmEncoder(new PCMEncoder(options.sampleRate, channels, channelLayout));
            pcmEncoder->InitEncoder();
            pcmEncoders.push_back(pcmEncoder);

            wavData.push_back(std::vector<uint8_t>());

            std::string audioFilePath = GetAudioFileName(options, channels, 24, i);
            if (filesystem::exists(audioFilePath)) {
                if (options.overwriteFiles) {
                    const filesystem::path path(audioFilePath);
                    filesystem::remove(path);
                } else {
                    std::cerr << audioFilePath << " exists already. Run with -f to overwrite!" << std::endl;
                    return 1;
                }
            }

            audioFiles.push_back(audioFilePath);
        }

        decoder.Decode([&] (RawVideoFrame &rawFrame) { return HandleVideoFrame(rawFrame, j2kEncoder, j2kFiles, options.tempFilePath); },
                       [&] (RawAudioFrame &rawFrame, int index) { return HandleAudioFrame(rawFrame, *(pcmEncoders[index]), wavData[index], index); });

        if (decoder.HasVideoTrack() && j2kFiles.empty() == false) {
            MXFWriter::MXFOptionsVideo videoOptions;


            videoOptions.editRate = options.editRate;
            // Aspect Ratio is now known.
            videoOptions.aspectRatio = decoder.GetAspectRatio();
            videoOptions.containerDuration = (uint32_t)(j2kFiles.size());
            videoOptions.yuvEssence = options.colorFormat != COLOR_FORMAT::CF_RGB444;
            videoOptions.subsamplingDx = options.colorFormat == CF_YUV422 ? 2 : 1;
            videoOptions.encryptHeader = false;
            videoOptions.bits = (int)(options.bitsPerComponent);
            videoOptions.broadcastProfile = (int)(options.profile);
            videoOptions.fullRange = options.fullRange;

            // write video
            MXFWriter videoMxfWriter;
            videoMxfWriter.MuxVideoFiles(j2kFiles, finalVideoFile, videoOptions);
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

            MXFWriter::MXFOptionsAudio audioOptions;
            audioOptions.editRate = RationalNumber(sampleRate, 1);

            MXFWriter audioMxfWriter;
            audioMxfWriter.MuxAudioFile(wavFileName, audioFiles[i], audioOptions);
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
