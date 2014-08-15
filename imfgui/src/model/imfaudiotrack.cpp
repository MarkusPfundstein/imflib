#include "imfaudiotrack.h"

IMFAudioTrack::IMFAudioTrack(const std::string& filename)
    :
    IMFTrack(filename, TYPE::AUDIO)
{
    //ctor
}

IMFAudioTrack::~IMFAudioTrack()
{
    //dtor
}
