#include "cplsequencerect.h"

#include "cplresourcerect.h"

CPLSequenceRect::CPLSequenceRect(QGraphicsItem *_parent)
    :
    CPLRenderRect(_parent),
    _trackIndex(0),
    _lastItem(nullptr)
{
    //ctor
}

CPLSequenceRect::~CPLSequenceRect()
{
    //dtor
}
