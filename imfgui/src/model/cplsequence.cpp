#include "cplsequence.h"

#include "cplresource.h"

CPLSequence::CPLSequence(const std::string &uuid)
    :
    GenericItem(uuid),
    _resources()
{
    //ctor
}

CPLSequence::~CPLSequence()
{
    //dtor
}

std::shared_ptr<CPLResource> CPLSequence::FindResourceByID(const std::string &uuid) const
{
    auto it = std::find_if(_resources.begin(),
                           _resources.end(),
                           [&uuid](const std::shared_ptr<CPLResource> &v) { return v->GetTrack()->GetUUID() == uuid; });
    if (it != _resources.end()) {
        return *it;
    }

    return std::shared_ptr<CPLResource>(nullptr);
}

void CPLSequence::AddResourceAfterResource(const std::shared_ptr<CPLResource> &resource, const std::shared_ptr<CPLResource> &other)
{
    auto it = std::find_if(_resources.begin(),
                           _resources.end(),
                           [&other](const std::shared_ptr<CPLResource> &v) { return v->GetTrack()->GetUUID() == other->GetTrack()->GetUUID(); });
    if (it != _resources.end()) {
        it++;
    }
    _resources.insert(it, resource);
}

void CPLSequence::AddResource(const std::shared_ptr<CPLResource> &resource)
{
    _resources.push_back(resource);
}

int CPLSequence::GetDuration() const
{
    int duration = 0;

    // SMPTE 2067-3
    // The duration of a sequence shall be the sum of the duration of its Resources and shall
    // be an integer number of Composition Edit Units
    for (const std::shared_ptr<CPLResource> &s : _resources) {
        duration += s->GetNormalizedSourceDuration();
    }

    return duration;
}
