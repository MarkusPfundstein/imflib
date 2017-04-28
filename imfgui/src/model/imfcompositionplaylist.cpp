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
    _editRate(0, 0),
    _segments(),
    _virtualTracks(),
    _header()
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

    rootNode.put("<xmlattr>.xmlns", "http://www.smpte-ra.org/schemas/2067-3/2013");
    rootNode.put("<xmlattr>.xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    rootNode.put("<xmlattr>.xmlns:imf", "http://www.smpte-ra.org/schemas/2067-2/2013");
    rootNode.put("Id", UUIDStr(GetUUID()));

    _header.Write(rootNode);

    ptree essenceDescriptorList;
/*
	xmlns:r0="http://www.smpte-ra.org/reg/395/2014/13/1/aaf"
        xmlns:r1="http://www.smpte-ra.org/reg/335/2012"
        xmlns:r2="http://www.smpte-ra.org/reg/2003/2012">
*/
    essenceDescriptorList.put("<xmlattr>.xmlns:r0", "http://www.smpte-ra.org/reg/395/2014/13/1/aaf");
    essenceDescriptorList.put("<xmlattr>.xmlns:r1", "http://www.smpte-ra.org/reg/335/2012");
    essenceDescriptorList.put("<xmlattr>.xmlns:r2", "http://www.smpte-ra.org/reg/2003/2012");

    // dump essence descriptorlist
    bool writeEssenceDescriptors = false;

    for (const std::shared_ptr<CPLSegment> &segment : _segments) {
	for (const std::shared_ptr<CPLSequence> &sequence : segment->GetItems()) {
	    for(const std::shared_ptr<CPLResource> &resource : sequence->GetItems()) {
		const IMFEssenceDescriptor &essenceDescriptor = resource->GetTrack()->GetEssenceDescriptor();
		std::cout << "----> write essence descriptor: " << essenceDescriptor.GetUUID() << std::endl;
		writeEssenceDescriptors = true;

		ptree ptED;
		ptED.put("Id", UUIDStr(essenceDescriptor.GetUUID()));

		ptree descRoot;
		descRoot.put(
		    "r1:SampleRate", 
	   	    boost::lexical_cast<std::string>(resource->GetTrack()->GetEditRate().num) + "/" + boost::lexical_cast<std::string>(resource->GetTrack()->GetEditRate().denum));
		ptED.add_child("r0:" + essenceDescriptor.GetTypeAsString(), descRoot);
		
		essenceDescriptorList.add_child("EssenceDescriptor", ptED);
	    }
	}	
    }

    if (writeEssenceDescriptors) {
        rootNode.add_child("EssenceDescriptorList", essenceDescriptorList);
    }

    rootNode.put("EditRate", boost::lexical_cast<std::string>(_editRate.num) + " " + boost::lexical_cast<std::string>(_editRate.denum));

    ptree extensionProperties;
    extensionProperties.put("imf:ApplicationIdentification", "http://www.smpte-ra.org/schemas/2067-21/2016");
    rootNode.add_child("ExtensionProperties", extensionProperties);

    ptree segmentList;

    // write segments list
    for (const std::shared_ptr<CPLSegment> &segment : _segments) {
        segment->Write(segmentList);
    }

    rootNode.add_child("SegmentList", segmentList);
    pt.add_child("CompositionPlaylist", rootNode);

    boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
    write_xml(GetPath(), pt, std::locale(), settings);
}

void IMFCompositionPlaylist::AddSegment(const std::shared_ptr<CPLSegment> &segment)
{
    _segments.push_back(segment);
}

void IMFCompositionPlaylist::InsertSegmentAfter(const std::shared_ptr<CPLSegment> &segment, const std::shared_ptr<CPLSegment> &other)
{
    auto it = std::find_if(_segments.begin(),
                            _segments.end(),
                            [&other](const std::shared_ptr<CPLSegment> &v) { return v->GetUUID() == other->GetUUID(); });
    //std::cout << (*--it)->GetUUID() << " - " << other->GetUUID() << std::endl;
    if (it != _segments.end()) {
        it++;
    }
    _segments.insert(it, segment);
}

std::shared_ptr<CPLVirtualTrack> IMFCompositionPlaylist::GetVirtualTrackAtIndex(unsigned int index) const
{
    std::cout << "index: " << index << " size: " << _virtualTracks.size() << std::endl;
    if (index < 0 || index >= _virtualTracks.size()) {
        std::cout << "return nullptr" << std::endl;
        return std::shared_ptr<CPLVirtualTrack>(nullptr);
    }
    return *std::next(_virtualTracks.begin(), index);
}

void IMFCompositionPlaylist::InsertSegmentBefore(const std::shared_ptr<CPLSegment> &segment, const std::shared_ptr<CPLSegment> &other)
{
    auto it = std::find_if(_segments.begin(),
                            _segments.end(),
                            [&other](const std::shared_ptr<CPLSegment> &v) { return v->GetUUID() == other->GetUUID(); });
    _segments.insert(it, segment);
}

void IMFCompositionPlaylist::DeleteSegment(const std::shared_ptr<CPLSegment> &segment)
{
    _segments.remove_if([&segment](const std::shared_ptr<CPLSegment> &s) { return s->GetUUID() == segment->GetUUID(); });
}

void IMFCompositionPlaylist::AddVirtualTrack(const std::shared_ptr<CPLVirtualTrack> &vt)
{
    _virtualTracks.push_back(vt);
}

void IMFCompositionPlaylist::DeleteVirtualTrack(const std::shared_ptr<CPLVirtualTrack> &vt)
{
    _virtualTracks.remove_if([&vt](const std::shared_ptr<CPLVirtualTrack> &v) { return v->GetUUID() == vt->GetUUID();});
}

bool IMFCompositionPlaylist::VirtualTrackExists(const std::string &id) const
{
    return _virtualTracks.end() != std::find_if(_virtualTracks.begin(),
                                                 _virtualTracks.end(),
                                                 [&id](const std::shared_ptr<CPLVirtualTrack> &v) { return v->GetUUID() == id; });
}

std::shared_ptr<CPLVirtualTrack> IMFCompositionPlaylist::FindVirtualTrackById(const std::string &id) const
{
    auto it = std::find_if(_virtualTracks.begin(),
                           _virtualTracks.end(),
                           [&id](const std::shared_ptr<CPLVirtualTrack> &v) { return v->GetUUID() == id; });
    if (it != _virtualTracks.end()) {
        return *it;
    }

    return std::shared_ptr<CPLVirtualTrack>(nullptr);
}

std::shared_ptr<IMFCompositionPlaylist> IMFCompositionPlaylist::Load(const std::string &path,
                                                                     boost::property_tree::ptree &pt,
                                                                     const std::vector<std::shared_ptr<IMFTrack>> &tracks)
{
    using namespace boost::property_tree;

    std::string cplUUID = pt.get<std::string>("CompositionPlaylist.Id");
    UUIDClean(cplUUID);

    std::shared_ptr<IMFCompositionPlaylist> playlist(new IMFCompositionPlaylist(cplUUID, path));

    ptree &headerNode = pt.get_child("CompositionPlaylist");
    playlist->_header.Read(headerNode);
    playlist->_header.Dump();

    std::string cplEditRate = pt.get<std::string>("CompositionPlaylist.EditRate");
    playlist->SetEditRate(RationalNumber::FromIMFString(cplEditRate));

    /* TO-DO: Parse Locale stuff */

    // parse each segment and resources. here we build our actual playlist.
    // segments get played after each other
    for (ptree::value_type const &segmentNode : pt.get_child("CompositionPlaylist.SegmentList")) {
        // id of segment
        std::string segmentUUID = segmentNode.second.get<std::string>("Id");
        UUIDClean(segmentUUID);

        std::string annotation = segmentNode.second.get<std::string>("Annotation", "");

        std::cout << "\tSegmentUUID: " << segmentUUID << std::endl;
        std::cout << "\tAnnotation: " << annotation << std::endl;

        std::shared_ptr<CPLSegment> segment(new CPLSegment(segmentUUID));

        playlist->AddSegment(segment);

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

            std::shared_ptr<CPLSequence> sequence(new CPLSequence(sequenceId));
            sequence->SetVirtualTrackId(virtualTrackId);

            segment->AppendItem(sequence);

            std::shared_ptr<CPLVirtualTrack> virtualTrack = playlist->FindVirtualTrackById(virtualTrackId);
            if (virtualTrack == nullptr) {
                virtualTrack.reset(new CPLVirtualTrack(virtualTrackId));
                playlist->AddVirtualTrack(virtualTrack);
                std::cout << "-> New Virtual Track generated" << std::endl;
            }

            virtualTrack->AppendItem(sequence);

            for (ptree::value_type const &resourceListNode : sequenceListNode.second.get_child("ResourceList")) {
                if (resourceListNode.first == "Resource") {
                    std::string resourceType = resourceListNode.second.get<std::string>("<xmlattr>.xsi:type");

                    // we only handle TrackFileResourceTypes now. This is AUDIO/VIDEO and SUBTITLES
                    if (resourceType.find("TrackFileResourceType") != std::string::npos) {
                        try {
                            std::shared_ptr<CPLResource> resource = CPLResource::Load(resourceListNode.second,
                                                                                      cplEditRate,
                                                                                      tracks);
                            //playlist->_essenceDescriptors.insert(resource->GetSourceEncoding());
                            sequence->AppendItem(resource);
                        } catch (IMFInvalidReferenceException &e) {
                            throw IMFCompositionPlaylistException(e.what());
                        }

                    }
                }
            }
        }
   }

    return playlist;
}

double IMFCompositionPlaylist::GetDurationInEditUnits() const
{
    return GetDurationInFrames() / _editRate.AsDouble();
}

int IMFCompositionPlaylist::GetDurationInFrames() const
{
    int duration = 0;

    // SMPTE 2067-3
    // The duration of the composition shall be the sum of the duration of
    // its segments
    for (const std::shared_ptr<CPLSegment> &s : _segments) {
        duration += s->GetDuration();
    }

    return duration;
}
