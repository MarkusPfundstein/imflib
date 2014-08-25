#include "imfpackage.h"

#include "imfvideotrack.h"
#include "imfaudiotrack.h"
#include "imfcompositionplaylist.h"
#include "imfoutputprofile.h"

#include "../utils/uuidgenerator.h"
#include "../utils/mxfreader.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/exceptions.hpp>

IMFPackage::IMFPackage()
    :
    _name(""),
    _saved(false),
    _location(""),
    _uuid(""),
    _videoTracks(),
    _audioTracks(),
    _compositionPlaylists(),
    _outputProfiles(),
    _headerAssetMap()
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

void IMFPackage::AddCompositionPlaylist(const std::shared_ptr<IMFCompositionPlaylist> &track)
{
    std::cout << "add composition playlist: " << track->GetFileName() << std::endl;
    _compositionPlaylists.push_back(track);
}

bool IMFPackage::HasTrackFile(const std::string &file) const
{
    return HasVideoTrackFile(file) || HasAudioTrackFile(file);
}

bool IMFPackage::HasVideoTrackFile(const std::string &file) const
{
    auto it = std::find_if(_videoTracks.begin(), _videoTracks.end(), [&file](const std::shared_ptr<IMFVideoTrack> &vt) { return vt->GetPath() == file; });
    return it != _videoTracks.end();
}

bool IMFPackage::HasAudioTrackFile(const std::string &file) const
{
    auto it = std::find_if(_audioTracks.begin(), _audioTracks.end(), [&file](const std::shared_ptr<IMFAudioTrack> &vt) { return vt->GetPath() == file; });
    return it != _audioTracks.end();
}

void IMFPackage::CopyTrackFiles() const
{
    using namespace boost::filesystem;

    for (std::shared_ptr<IMFVideoTrack> asset : _videoTracks) {

        std::string newLocation = _location + "/" + _name + "/" + asset->GetFileName();

        path src(asset->GetPath());
        path target(newLocation);

        if (exists(target) == false) {
            std::cout << "Copy " << asset->GetFileName() << " to package.... can take a bit..." << std::endl;
            copy_file(src, target);
            asset->SetPath(target.string());
        }
    }

    for (std::shared_ptr<IMFAudioTrack> asset : _audioTracks) {
        std::string newLocation = _location + "/" + _name + "/" + asset->GetFileName();

        path src(asset->GetPath());
        path target(newLocation);

        if (exists(target) == false) {
            std::cout << "Copy " << asset->GetFileName() << " to package.... can take a bit..." << std::endl;
            try {
                copy_file(src, target);
            } catch (boost::filesystem::filesystem_error &e) {
                throw IMFPackageException(e.what());
            }
            asset->SetPath(target.string());
        }
    }

    std::cout << "Copying Done" << std::endl;
}

void IMFPackage::Load(const std::string &directory)
{
    using namespace boost::filesystem;
    path dirPath(directory);

    SetLocation(dirPath.parent_path().string());
    SetName(dirPath.filename().string());

    std::cout << "Open: " << GetLocation() << std::endl;
    std::cout << "Package: " << GetName() << std::endl;

    // yes, this could be directory + "/ASSETMAP" but i wont it to be like Write()
    ReadAssetMap(_location + "/" + _name + "/ASSETMAP.xml");
}

void IMFPackage::ReadAssetMap(const std::string& filename)
{
    using namespace boost::property_tree;
    using namespace boost::filesystem;

    try {

        // we put all xmlFiles in one vector before we parse them
        // because we first need all track files.
        std::vector<std::string> xmlFiles;

        ptree pt;
        read_xml(filename, pt);

        std::string packageId = pt.get<std::string>("AssetMap.Id");
        UUIDClean(packageId);
        SetUUID(packageId);

        std::cout << "UUID: " << packageId << std::endl;

        ptree &headerChild = pt.get_child("AssetMap");
        _headerAssetMap.Read(headerChild);

        _headerAssetMap.Dump();

        for (ptree::value_type const &assetChild : pt.get_child("AssetMap.AssetList")) {

            if (assetChild.first == "Asset") {

                std::cout << "Asset:" << std::endl;

                std::string assetId = assetChild.second.get<std::string>("Id");
                UUIDClean(assetId);
                std::cout << "\tUUID: " << assetId << std::endl;
                for (ptree::value_type const &chunkListChild : assetChild.second.get_child("ChunkList")) {

                    std::string name = chunkListChild.second.get<std::string>("Path");
                    int volumeIndex = chunkListChild.second.get<int>("VolumeIndex");

                    //boost::optional<int> offset = chunkListChild.second.get_optional<int>("Offset");
                    //boost::optional<int> length = chunkListChild.second.get_optional<int>("Length");

                    std::cout << "\tName: " << name << std::endl;
                    std::cout << "\tVolumeIndex: " << volumeIndex << std::endl;

                    std::string fullPath = _location + "/" + _name + "/" + name;
                    if (boost::filesystem::path(name).extension().string() == ".mxf") {
                        ParseAndAddTrack(fullPath);
                    } else if (boost::filesystem::path(name).extension().string() == ".xml") {
                        xmlFiles.push_back(fullPath);
                    }
                }
            }
        }

        // done with parsing list. now lets check our xml files
        for (const std::string &s : xmlFiles) {
            ParseAndAddXML(s);
        }
    } catch (xml_parser::xml_parser_error &e) {
        throw IMFPackageException(e.what());
    } catch (ptree_bad_path &e) {
        throw IMFPackageException(e.what());
    } catch (IMFCompositionPlaylistException &e) {
        throw IMFPackageException(e.what());
    }
}

void IMFPackage::ParseAndAddXML(const std::string& fullPath)
{
    using namespace boost::property_tree;

    std::cout << "Parse And Add XML: " << fullPath << std::endl;

    // parse root node to check what type of file we have

    ptree pt;
    try {
        read_xml(fullPath, pt);

        // only way to figure out if xml is composition playlist. we check if root is it
        boost::optional<std::string> opt = pt.get_optional<std::string>("CompositionPlaylist");
        if (opt) {
            std::cout << "Got composition playlist" << std::endl;

            /* make vector of all our tracks */
            std::vector<std::shared_ptr<IMFTrack>> tracks;
            tracks.insert(tracks.end(), _videoTracks.begin(), _videoTracks.end());
            tracks.insert(tracks.end(), _audioTracks.begin(), _audioTracks.end());

            std::shared_ptr<IMFCompositionPlaylist> playlist = IMFCompositionPlaylist::Load(fullPath, pt, tracks);
            AddCompositionPlaylist(playlist);
            return;
        }
        opt = pt.get_optional<std::string>("OutputProfile");
        if (opt) {
            std::cout << "Got output profile" << std::endl;
        }

    } catch (xml_parser::xml_parser_error &e) {
        throw IMFPackageException(e.what());
    } catch (ptree_bad_path &e) {
        throw IMFPackageException(e.what());
    }
}

void IMFPackage::ParseAndAddTrack(const std::string& fullPath)
{
    std::cout << "Parse And Add A/V Track: " << fullPath << std::endl;
    MXFReader mxfReader(fullPath);
    try {
        MXFReader::ESSENCE_TYPE essenceType = mxfReader.GetEssenceType();

        if (essenceType == MXFReader::ESSENCE_TYPE::VIDEO) {
            // parse header to get metadata for videotrack
            std::shared_ptr<IMFVideoTrack> videoTrack = mxfReader.ReadVideoTrack();
            AddVideoTrack(videoTrack);
        } else if (essenceType == MXFReader::ESSENCE_TYPE::AUDIO) {
            // parse header to get metadata for videotrack
            std::shared_ptr<IMFAudioTrack> audioTrack = mxfReader.ReadAudioTrack();
            AddAudioTrack(audioTrack);
        } else {
            throw IMFPackageException("invalid essence type");
        }
    } catch (MXFReaderException &ex) {
        throw IMFPackageException(ex.what());
    }
}

void IMFPackage::Write() const
{
    try {
        // write composition playlists
        for (const std::shared_ptr<IMFCompositionPlaylist> &cpl : _compositionPlaylists) {
            cpl->Write();
        }

        // last but not least. write map of all assets
        WriteAssetMap(_location + "/" + _name + "/ASSETMAP.xml");
    } catch (IMFCompositionPlaylistException &e) {
        throw IMFPackageException(e.what());
    }
}

void IMFPackage::WriteAssetMap(const std::string& filename) const
{
    using boost::property_tree::ptree;
    ptree pt;

    ptree rootNode;

    rootNode.put("<xmlattr>.xmlns", "http://www.smpte-ra.org/schemas/429-9/2007/AM");
    rootNode.put("Id", UUIDStr(_uuid));

    _headerAssetMap.Write(rootNode);

    ptree assetListNode;

    // merge all assets into one big array
    std::vector<std::shared_ptr<IMFPackageItem>> assets;
    assets.insert(assets.end(), _videoTracks.begin(), _videoTracks.end());
    assets.insert(assets.end(), _audioTracks.begin(), _audioTracks.end());
    assets.insert(assets.end(), _compositionPlaylists.begin(), _compositionPlaylists.end());
    assets.insert(assets.end(), _outputProfiles.begin(), _outputProfiles.end());

    for (const std::shared_ptr<IMFPackageItem>& item : assets) {
        ptree chunkNode;
        chunkNode.put("Path", item->GetFileName());
        chunkNode.put("VolumeIndex", "1");
        chunkNode.put("Offset", "0");
        chunkNode.put("Length", boost::lexical_cast<std::string>(item->GetFileSize()));

        ptree chunkListNode;
        chunkListNode.add_child("Chunk", chunkNode);

        ptree assetNode;
        assetNode.put("Id", UUIDStr(item->GetUUID()));
        assetNode.add_child("ChunkList", chunkListNode);

        assetListNode.add_child("Asset", assetNode);
    }

    rootNode.add_child("AssetList", assetListNode);
    pt.add_child("AssetMap", rootNode);

    boost::property_tree::xml_writer_settings<char> settings('\t', 1);
    write_xml(filename, pt, std::locale(), settings);

    return;
}
