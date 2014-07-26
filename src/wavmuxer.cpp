#include "wavmuxer.h"

#include <fstream>
#include <iostream>

WavMuxer::WavMuxer()
{
    //ctor
}

WavMuxer::~WavMuxer()
{
    //dtor
}

void WavMuxer::MuxToFile(const std::string &filename, const std::vector<uint8_t> &data, short channels, int sampleRate, short bitsPerSample) const
{
    std::ofstream file(filename.c_str(), std::ios::binary);

    char chunkId[] = {'R', 'I', 'F', 'F'};
    unsigned int chunkSize = 36 + data.size();
    char format[] = {'W', 'A', 'V', 'E'};

    // Write the first header
    file.write(chunkId, 4);
    file.write((char*)&chunkSize, 4);
    file.write(format, 4);

    chunkId[0] = 'f'; chunkId[1] = 'm'; chunkId[2] = 't'; chunkId[3] = ' ';
    chunkSize = 16;
    short audioFormat = 1;
    short bytesPerSample = bitsPerSample / 8;
    int byteRate = channels * sampleRate * bytesPerSample;
    short blockAlign = channels * bytesPerSample;

    std::cout << "channels: " << channels << std::endl;
    std::cout << "sampleRate: " << sampleRate << std::endl;
    std::cout << "byteRate: " << byteRate << std::endl;
    std::cout << "blockAlign: " << blockAlign << std::endl;
    std::cout << "bitsPerSample: " << bitsPerSample << std::endl;
    std::cout << "bytesPerSample: " << bytesPerSample << std::endl;

    // Write the second header
    file.write(chunkId, 4);
    file.write((char*)&chunkSize, 4);
    file.write((char*)&audioFormat, 2);
    file.write((char*)&channels, 2);
    file.write((char*)&sampleRate, 4);
    file.write((char*)&byteRate, 4);
    file.write((char*)&blockAlign, 2);
    file.write((char*)&bitsPerSample, 2);

    chunkId[0] = 'd'; chunkId[1] = 'a'; chunkId[2] = 't'; chunkId[3] = 'a';
    chunkSize = data.size();

    // Write the third header and the actual data
    file.write(chunkId, 4);
    file.write((char*)&chunkSize, 4);
    file.write((char*)&data[0], data.size());

    file.close();
}
