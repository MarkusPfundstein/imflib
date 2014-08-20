#include "cplsegmentrect.h"

#include <QtGui>
#include <iostream>

CPLSegmentRect::CPLSegmentRect(QGraphicsItem *_parent)
    :
    CPLRenderRect(_parent),
    _index(0)
{
    //ctor
}

CPLSegmentRect::~CPLSegmentRect()
{
    //dtor
}

void CPLSegmentRect::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    std::cout << "SegmentRect pressed: " << ev->pos().x() << std::endl;
    QGraphicsItem::mousePressEvent(ev);
}
