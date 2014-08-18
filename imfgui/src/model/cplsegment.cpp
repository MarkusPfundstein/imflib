#include "cplsegment.h"

#include "cplsequence.h"

#include <iostream>

CPLSegment::CPLSegment(const std::string &uuid)
    :
    GenericItem(uuid),
    _sequences()
{
    //ctor
}

CPLSegment::~CPLSegment()
{
    //dtor
}

void CPLSegment::AddSequence(const std::shared_ptr<CPLSequence> &sequence)
{
    _sequences.push_back(sequence);
}

int CPLSegment::GetDuration() const
{
    int duration = 0;

    // SMPTE 2067-3
    // The duration of the segment shall be equal to the duration of its sequences
    // and all sequences withing a segment shall have the same duration
    if (_sequences.empty() == false) {
        duration = _sequences[0]->GetDuration();
    }

    return duration;
}
