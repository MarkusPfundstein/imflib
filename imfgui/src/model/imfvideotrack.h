#ifndef IMFVIDEOTRACK_H
#define IMFVIDEOTRACK_H

#include "imftrack.h"

class IMFVideoTrack : public IMFTrack
{
    public:
        IMFVideoTrack(const std::string& filename);
        virtual ~IMFVideoTrack();

    protected:
    private:
};

#endif // IMFVIDEOTRACK_H
