#include "imfaudiotrack.h"

IMFAudioTrack::IMFAudioTrack(const std::string &uuid, const std::string& filename, std::shared_ptr<IMFEssenceDescriptor> essenceDescriptor)
    :
    IMFTrack(uuid, filename, TYPE::AUDIO, essenceDescriptor)
{
    //ctor
}

IMFAudioTrack::~IMFAudioTrack()
{
    //dtor
}
