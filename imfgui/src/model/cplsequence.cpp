#include "cplsequence.h"

#include "cplresource.h"

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
