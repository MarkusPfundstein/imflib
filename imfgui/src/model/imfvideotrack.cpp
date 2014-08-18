#include "imfvideotrack.h"

IMFVideoTrack::IMFVideoTrack(const std::string &uuid, const std::string& filename)
    :
    IMFTrack(uuid, filename, TYPE::VIDEO),
    _colorSpace(IMF_COLOR_SPACE::INVALID)
{
    //ctor
}

IMFVideoTrack::~IMFVideoTrack()
{
    //dtor
}

std::string IMFVideoTrack::TypeString() const
{
    switch (_colorSpace) {
        case RGB444:
            return "RGB444";
        case YUV444:
            return "YUV444";
        case YUV422:
            return "YUV422";
        case INVALID:
        default:
            return "INVALID";
    }
}
