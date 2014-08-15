#include "imfpackage.h"

#include "imfvideotrack.h"
#include "imfaudiotrack.h"
#include "imfcompositionplaylist.h"
#include "imfoutputprofile.h"
#include "../utils/xmlwriter.h"

#include <iostream>
#include <fstream>
#include <algorithm>

IMFPackage::IMFPackage()
    :
    _name(""),
    _saved(false),
    _location(""),
    _uuid(""),
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
    std::cout << "add video track: " << track->GetFileName() << std::endl;
    _videoTracks.push_back(track);
}

void IMFPackage::AddAudioTrack(const std::shared_ptr<IMFAudioTrack> &track)
{
    std::cout << "add audio track: " << track->GetFileName() << std::endl;
    _audioTracks.push_back(track);
}

bool IMFPackage::HasTrackFile(const std::string &file) const
{
    return HasVideoTrackFile(file) || HasAudioTrackFile(file);
}

bool IMFPackage::HasVideoTrackFile(const std::string &file) const
{
    auto it = std::find_if(_videoTracks.begin(), _videoTracks.end(), [&file](const std::shared_ptr<IMFVideoTrack> &vt) { return vt->GetFileName() == file; });
    return it != _videoTracks.end();
}

bool IMFPackage::HasAudioTrackFile(const std::string &file) const
{
    auto it = std::find_if(_audioTracks.begin(), _audioTracks.end(), [&file](const std::shared_ptr<IMFAudioTrack> &vt) { return vt->GetFileName() == file; });
    return it != _audioTracks.end();
}

void IMFPackage::Write() const
{
    WriteAssetMap(_location + "/" + _name + "/ASSETMAP.xml");
}

void IMFPackage::WriteAssetMap(const std::string& filename) const
{
    using namespace xmlw;

    std::ofstream assetMapStream(filename, std::ios::out);

    XmlStream stream(assetMapStream);

    stream << prolog()
        << tag("AssetMap")
            << attr("xmlns") << "http://www.smpte-ra.org/schemas/429-9/2007/AM"
            << tag("Id")
                << chardata() << "urn:uuid:" << _uuid
            << endtag()
        << endtag();
}
