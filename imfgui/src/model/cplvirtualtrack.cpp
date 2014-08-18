#include "cplvirtualtrack.h"

#include "cplsequence.h"

CPLVirtualTrack::CPLVirtualTrack(const std::string &uuid)
    :
    GenericItem(uuid),
    _sequences()
{
    //ctor
}

CPLVirtualTrack::~CPLVirtualTrack()
{
    //dtor
}

void CPLVirtualTrack::AddSequence(const std::shared_ptr<CPLSequence> &sequence)
{
    _sequences.push_back(sequence);
}

