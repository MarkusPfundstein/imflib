#include "cplsequencerect.h"

#include "cplresourcerect.h"
#include "../model/cplsequence.h"

CPLSequenceRect::CPLSequenceRect(QGraphicsItem *_parent, const std::shared_ptr<CPLSequence> &sequence)
    :
    CPLRenderRect(_parent),
    _sequence(sequence),
    _trackIndex(0),
    _lastItem(nullptr),
    _firstItem(nullptr)
{
    //ctor
}

CPLSequenceRect::~CPLSequenceRect()
{
    //dtor
}
