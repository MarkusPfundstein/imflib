#include "imfaudiotrack.h"

IMFAudioTrack::IMFAudioTrack(const std::string& filename)
    :
    IMFTrack(filename)
{
    //ctor
}

IMFAudioTrack::~IMFAudioTrack()
{
    //dtor
}

IMFAudioTrack::IMFAudioTrack(const IMFAudioTrack& other)
    :
    IMFTrack(other._filename)
{
    //copy ctor
}

IMFAudioTrack& IMFAudioTrack::operator=(const IMFAudioTrack& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}
