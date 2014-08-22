#include "cplsegmentrect.h"

#include <QtGui>
#include <iostream>

CPLSegmentRect::CPLSegmentRect(QGraphicsItem *_parent, const std::shared_ptr<CPLSegment> &segment)
    :
    CPLRenderRect(_parent),
    CPLPackageItemContainer(segment),
    _index(0)
{
    //ctor
    SetAcceptAttachment(true);
}

CPLSegmentRect::~CPLSegmentRect()
{
    //dtor
}

void CPLSegmentRect::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    std::cout << "SegmentRect pressed: " << ev->pos().x() << std::endl;
    if (ev->button() == Qt::RightButton) {
        ev->accept();
        emit RightMouseClickSignal(ev->pos(), *this);
    }
}

#include "../moc_cplsegmentrect.cpp"
