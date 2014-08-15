#include "imfvideotrack.h"

IMFVideoTrack::IMFVideoTrack(const std::string& filename)
    :
    IMFTrack(filename, TYPE::VIDEO)
{
    //ctor
}

IMFVideoTrack::~IMFVideoTrack()
{
    //dtor
}
