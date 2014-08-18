#include "imfcompositionplaylist.h"

#include "../utils/uuidgenerator.h"

#include "cplresource.h"
#include "cplsegment.h"
#include "cplsequence.h"
#include "cplvirtualtrack.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/exceptions.hpp>

#include <algorithm>

IMFCompositionPlaylist::IMFCompositionPlaylist(const std::string &uuid, const std::string& filename)
    :
    IMFPackageItem(uuid, filename, IMFPackageItem::TYPE::CPL),
    _editRate(0, 0)
{
    //ctor
}

IMFCompositionPlaylist::~IMFCompositionPlaylist()
{
    //dtor
}

void IMFCompositionPlaylist::Write() const
{
    using boost::property_tree::ptree;
    ptree pt;

    ptree rootNode;

    rootNode.put("<xmlattr>.xmlns", "http://www.smpte-ra.org/schemas/2067-3/XXXX");
    rootNode.put("<xmlattr>.xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    rootNode.put("Id", UUIDStr(GetUUID()));
    rootNode.put("Annotation", XML_HEADER_ANNOTATION);
    rootNode.put("Creator", XML_HEADER_CREATOR);
    rootNode.put("IssueDate", "");
    rootNode.put("Issuer", XML_HEADER_ISSUER);
    rootNode.put("ContentOriginator", XML_HEADER_ISSUER);
    rootNode.put("ContentTitle", "TestPackage");
    rootNode.put("ContentKind", "Test");
    rootNode.put("EditRate", boost::lexical_cast<std::string>(_editRate.num) + " " + boost::lexical_cast<std::string>(_editRate.denum));

    ptree segmentList;

    // write segments list
    /*for (const std::shared_ptr<Segment> &segment : _segments) {

    }*/

    rootNode.add_child("SegmentList", segmentList);
    pt.add_child("CompositionPlaylist", rootNode);

    boost::property_tree::xml_writer_settings<char> settings('\t', 1);
    write_xml(GetPath(), pt, std::locale(), settings);
}

std::shared_ptr<IMFCompositionPlaylist> IMFCompositionPlaylist::Load(const std::string &path,
                                                                     boost::property_tree::ptree &pt,
                                                                     const std::vector<std::shared_ptr<IMFTrack>> &tracks)
{
    using namespace boost::property_tree;

    std::string cplUUID = pt.get<std::string>("CompositionPlaylist.Id");
    UUIDClean(cplUUID);

    std::shared_ptr<IMFCompositionPlaylist> playlist(new IMFCompositionPlaylist(cplUUID, path));

    std::string cplEditRate = pt.get<std::string>("CompositionPlaylist.EditRate");
    playlist->SetEditRate(RationalNumber::FromIMFString(cplEditRate));

    /* TO-DO: Parse Locale stuff */

    // parse each segment and resources. here we build our actual playlist.
    // segments get played after each other
    for (ptree::value_type const &segmentNode : pt.get_child("CompositionPlaylist.SegmentList")) {
        // id of segment
        std::string segmentUUID = segmentNode.second.get<std::string>("Id");
        UUIDClean(segmentUUID);

        std::cout << "\tSegmentUUID: " << segmentUUID << std::endl;

        // go through all sequences of segment. sequences get played synchrounously at the same time
        for (ptree::value_type const &sequenceListNode : segmentNode.second.get_child("SequenceList")) {
            // id of sequence
            std::string sequenceId = sequenceListNode.second.get<std::string>("Id");
            UUIDClean(sequenceId);

            // virtual track id.
            std::string virtualTrackId = sequenceListNode.second.get<std::string>("TrackId");
            UUIDClean(virtualTrackId);

            std::cout << "\t\tSequence Id: " << sequenceId << std::endl;
            std::cout << "\t\tVirtual Track Id: "<< virtualTrackId << std::endl;

            for (ptree::value_type const &resourceListNode : sequenceListNode.second.get_child("ResourceList")) {
                if (resourceListNode.first == "Resource") {
                    std::string resourceType = resourceListNode.second.get<std::string>("<xmlattr>.xsi:type");

                    // we only handle TrackFileResourceTypes now. This is AUDIO/VIDEO and SUBTITLES
                    if (resourceType.find("TrackFileResourceType") != std::string::npos) {
                        std::shared_ptr<CPLResource> resource = playlist->LoadCPLResource(resourceListNode.second, cplEditRate, tracks);
                    }
                }
            }
        }
   }

    return playlist;
}

std::shared_ptr<CPLResource> IMFCompositionPlaylist::LoadCPLResource(const boost::property_tree::ptree &pt,
                                                                     const std::string &cplEditRate,
                                                                     const std::vector<std::shared_ptr<IMFTrack>> &tracks)
{
    std::string resourceId = pt.get<std::string>("Id");
    UUIDClean(resourceId);

    // real duration of essence in frames
    int intrinsicDuration = pt.get<int>("IntrinsicDuration");

    // entry point of essence. defaults to 0
    int entryPoint = pt.get<int>("EntryPoint", 0);

    // playing duration of essence. defaults to intrinsic - entrypoint
    int sourceDuration = pt.get<int>("SourceDuration", intrinsicDuration - entryPoint);

    // how often shall essence be played in thus sequence, defaults to 1
    int repeatCount = pt.get<int>("RepeatCount", 1);
    std::string editRateString = pt.get<std::string>("EditRate", cplEditRate);
    RationalNumber resourceEditRate = RationalNumber::FromIMFString(editRateString);

    // refers to EssenceDescriptorList in CPL
    // but no idea yet to what field in MXF file
    std::string sourceEncodingId = pt.get<std::string>("SourceEncoding");
    UUIDClean(sourceEncodingId);

    // refers to tracks in ASSETMAP, thus in imfpackage
    std::string trackFileId = pt.get<std::string>("TrackFileId");
    UUIDClean(trackFileId);

    // key to decrypt essence
    std::string keyId = pt.get<std::string>("KeyId", "");

    // hash to validate essence
    std::string hash = pt.get<std::string>("Hash", "");

    // search track in tracks
    auto it = std::find_if(tracks.begin(), tracks.end(), [&trackFileId](const std::shared_ptr<IMFTrack>& t) { return t->GetUUID() == trackFileId; });
    if (it == tracks.end()) {
        throw IMFCompositionPlaylistException("Track referenced in CPL which is not defined in ASSETMAP");
    }

    std::shared_ptr<CPLResource> cplResource(new CPLResource(resourceId, *it));
    cplResource->SetEntryPoint(entryPoint);
    cplResource->SetSourceDuration(sourceDuration);
    cplResource->SetRepeatCount(repeatCount);
    cplResource->SetSourceEncoding(sourceEncodingId);
    cplResource->SetKeyId(keyId);
    cplResource->SetHash(hash);

    std::cout << "\t\t\tResource ID: " << resourceId << std::endl;
    std::cout << "\t\t\tIntrinsic Duration: " << intrinsicDuration << std::endl;
    std::cout << "\t\t\tEntry Point: " << entryPoint << std::endl;
    std::cout << "\t\t\tSource Duration: " << sourceDuration << std::endl;
    std::cout << "\t\t\tRepeat Count: " << repeatCount << std::endl;
    std::cout << "\t\t\tEdit Rate: " << resourceEditRate.AsIMFString() << std::endl;
    std::cout << "\t\t\tSourceEncoding: " << sourceEncodingId << std::endl;
    std::cout << "\t\t\tTrackFileId: " << trackFileId << std::endl;
    std::cout << "\t\t\tKeyId: " << keyId << std::endl;
    std::cout << "\t\t\tHash: " << hash << std::endl;

    return cplResource;
}
