#include "cplsequence.h"

#include "cplresource.h"

#include "../utils/uuidgenerator.h"

CPLSequence::CPLSequence(const std::string &uuid)
    :
    GenericItem(uuid)
{
    //ctor
}

CPLSequence::~CPLSequence()
{
    //dtor
}

int CPLSequence::GetDuration() const
{
    int duration = 0;

    // SMPTE 2067-3
    // The duration of a sequence shall be the sum of the duration of its Resources and shall
    // be an integer number of Composition Edit Units
    for (const std::shared_ptr<CPLResource> &s : _items) {
        duration += s->GetNormalizedSourceDuration();
    }

    return duration;
}

void CPLSequence::Write(boost::property_tree::ptree &pt) const
{
    using namespace boost::property_tree;

    ptree sequence;

    sequence.put("Id", UUIDStr(GetUUID()));
    sequence.put("TrackId", UUIDStr(GetVirtualTrackId()));

    bool allVideo = true;
    bool allAudio = true;

    bool hasItem = false;

    ptree resourceList;

    for (const std::shared_ptr<CPLResource> &s : GetItems()) {
        hasItem = true;
        if (s->GetTrack()->GetType() == IMFPackageItem::TYPE::VIDEO) {
            allAudio = false;
        }
        if (s->GetTrack()->GetType() == IMFPackageItem::TYPE::AUDIO) {
            allVideo = false;
        }
        s->Write(resourceList);
    }

    if (allAudio == false && allVideo == false) {
        throw IMFCompositionPlaylistException("Each Resource in Sequence (Virtual Track) must be of same type");
    }

    sequence.add_child("ResourceList", resourceList);

    if (hasItem && allAudio) {
        pt.add_child("MainAudioSequence", sequence);
    } else if (hasItem && allVideo) {
        pt.add_child("MainImageSequence", sequence);
    }
}
