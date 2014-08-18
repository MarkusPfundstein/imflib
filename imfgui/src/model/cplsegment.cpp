#include "cplsegment.h"

#include "cplsequence.h"

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
