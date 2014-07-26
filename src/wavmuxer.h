#ifndef WAVMUXER_H
#define WAVMUXER_H

#include <string>
#include <vector>

class WavMuxer
{
    public:
        WavMuxer();
        virtual ~WavMuxer();

        void MuxToFile(const std::string &file, const std::vector<uint8_t> &data, short channels, int sampleRate, short bitsPerSample) const;
    protected:
    private:
};

#endif // WAVMUXER_H
