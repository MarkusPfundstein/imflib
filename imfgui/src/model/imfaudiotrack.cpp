#include "imfaudiotrack.h"

IMFAudioTrack::IMFAudioTrack(const std::string &uuid, const std::string& filename)
    :
    IMFTrack(uuid, filename, TYPE::AUDIO)
{
    //ctor
}

IMFAudioTrack::~IMFAudioTrack()
{
    //dtor
}
