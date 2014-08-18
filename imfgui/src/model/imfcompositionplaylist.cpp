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
                        std::shared_ptr<CPLResource> resource = CPLResource::Load(resourceListNode.second, cplEditRate, tracks);
                    }
                }
            }
        }
   }

    return playlist;
}
