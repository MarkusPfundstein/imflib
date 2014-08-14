#ifndef IMFAUDIOTRACK_H
#define IMFAUDIOTRACK_H

#include "imftrack.h"


class IMFAudioTrack : public IMFTrack
{
    public:
        IMFAudioTrack(const std::string& filename);
        virtual ~IMFAudioTrack();
        IMFAudioTrack(const IMFAudioTrack& other);
        IMFAudioTrack& operator=(const IMFAudioTrack& other);
    protected:
    private:
};

#endif // IMFAUDIOTRACK_H
