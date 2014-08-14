#ifndef IMFVIDEOTRACK_H
#define IMFVIDEOTRACK_H

#include "imftrack.h"

class IMFVideoTrack : public IMFTrack
{
    public:
        IMFVideoTrack();
        virtual ~IMFVideoTrack();
        IMFVideoTrack(const IMFVideoTrack& other);
        IMFVideoTrack& operator=(const IMFVideoTrack& other);
    protected:
    private:
};

#endif // IMFVIDEOTRACK_H
