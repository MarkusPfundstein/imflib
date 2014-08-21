#include "cplsequencerect.h"

#include "cplresourcerect.h"
#include "../model/cplsequence.h"

#include <iostream>
#include <QGraphicsSceneMouseEvent>

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

void CPLSequenceRect::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    std::cout << "sequence pressed" << std::endl;
    ev->accept();
}
