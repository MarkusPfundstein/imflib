#ifndef IMFVIDEOTRACK_H
#define IMFVIDEOTRACK_H

#include "imftrack.h"

class IMFVideoTrack : public IMFTrack
{
    public:
        enum IMF_COLOR_SPACE {
            INVALID = 0,
            RGB444  = 1,
            YUV444  = 2,
            YUV422  = 3
        };

        IMFVideoTrack(const std::string &uuid, const std::string& filename);
        virtual ~IMFVideoTrack();

        std::string TypeString() const;

        IMF_COLOR_SPACE GetColorSpace() const
        { return _colorSpace; }

        void SetColorSpace(IMF_COLOR_SPACE colorSpace)
        { _colorSpace = colorSpace; }

    protected:
    private:

        IMF_COLOR_SPACE _colorSpace;
};

#endif // IMFVIDEOTRACK_H
