#ifndef WAVMUXER_H
#define WAVMUXER_H


class WAVMuxer
{
    public:
        WAVMuxer();

        WAVMuxer(const WAVMuxer&) = delete;
        WAVMuxer* operator=(const WAVMuxer&) = delete;

        virtual ~WAVMuxer();
    protected:
    private:
};

#endif // WAVMUXER_H
