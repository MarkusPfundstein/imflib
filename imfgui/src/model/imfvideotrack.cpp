#include "imfvideotrack.h"

IMFVideoTrack::IMFVideoTrack(const std::string& filename)
    :
    IMFTrack(filename)
{
    //ctor
}

IMFVideoTrack::~IMFVideoTrack()
{
    //dtor
}

IMFVideoTrack::IMFVideoTrack(const IMFVideoTrack& other)
    :
    IMFTrack(other._filename)
{
    //copy ctor
}

IMFVideoTrack& IMFVideoTrack::operator=(const IMFVideoTrack& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}
