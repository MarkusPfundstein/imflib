#ifndef PCMENCODER_H
#define PCMENCODER_H

struct AVFrame;

class PCMEncoder
{
    public:
        PCMEncoder();
        ~PCMEncoder();

        void InitEncoder();

        void EncodeRawFrame(const AVFrame &rawFrame);

    protected:
    private:
};

#endif // PCMENCODER_H
