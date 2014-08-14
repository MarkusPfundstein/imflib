#include "imfpackage.h"

#include "imfvideotrack.h"
#include "imfaudiotrack.h"
#include "imfcompositionplaylist.h"
#include "imfoutputprofile.h"

#include <iostream>

IMFPackage::IMFPackage()
    :
    _videoTracks(),
    _audioTracks(),
    _compositionPlaylists(),
    _outputProfiles()
{
    //ctor
    std::cout << "create new package" << std::endl;
}

IMFPackage::~IMFPackage()
{
    //dtor
    std::cout << "delete package" << std::endl;
}

void IMFPackage::AddVideoTrack(const std::shared_ptr<IMFVideoTrack> &track)
{
    _videoTracks.push_back(track);
}

void IMFPackage::AddAudioTrack(const std::shared_ptr<IMFAudioTrack> &track)
{
    _audioTracks.push_back(track);
}
