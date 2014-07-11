#ifndef J2KENCODER_H
#define J2KENCODER_H

// forward declarations
struct RawVideoFrame;

struct J2kFrame
{

};

class J2KEncoder
{
    public:
        J2KEncoder();
        virtual ~J2KEncoder();

        bool EncodeRawFrame(RawVideoFrame &rawVideoFrame, J2kFrame& encodedFrame);
};

#endif // J2KENCODER_H
