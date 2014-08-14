#include "imfaudiotrack.h"

IMFAudioTrack::IMFAudioTrack()
{
    //ctor
}

IMFAudioTrack::~IMFAudioTrack()
{
    //dtor
}

IMFAudioTrack::IMFAudioTrack(const IMFAudioTrack& other)
{
    //copy ctor
}

IMFAudioTrack& IMFAudioTrack::operator=(const IMFAudioTrack& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}
