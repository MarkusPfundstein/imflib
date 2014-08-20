#include "cplsegment.h"

#include "cplsequence.h"

#include <iostream>

CPLSegment::CPLSegment(const std::string &uuid)
    :
    GenericItem(uuid)
{
    //ctor
}

CPLSegment::~CPLSegment()
{
    //dtor
}

int CPLSegment::GetDuration() const
{
    int duration = 0;

    // SMPTE 2067-3
    // The duration of the segment shall be equal to the duration of its sequences
    // and all sequences withing a segment shall have the same duration
    // This implies that all sequences are of the same length, Unfortunately this
    // is not a tautology. So we just find and return the longest
    for (const std::shared_ptr<CPLSequence> &s : _items) {
        int d = s->GetDuration();
        if (d > duration) {
            duration = d;
        }
    }

    return duration;
}
