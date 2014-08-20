#include "cplrenderrect.h"

#include <QtGui>
#include <iostream>

static int DEBUG_COUNT = 0;

CPLRenderRect::CPLRenderRect(QGraphicsItem *_parent)
    :
    QGraphicsItem(_parent),
    _renderBackground(true),
    _drawingRect(0, 0, 0, 0),
    _color(16, 233, 177, 255)
{
    DEBUG_COUNT++;
}

CPLRenderRect::~CPLRenderRect()
{
    //dtor
    std::cout << "Delete renderrect [" << --DEBUG_COUNT << " left]" << std::endl;
}


QRectF CPLRenderRect::boundingRect() const
{
    return _drawingRect;

}

void CPLRenderRect::SetDrawingRect(const QRect &r)
{
    _drawingRect = r;
}

void CPLRenderRect::paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget)
{
    if (_renderBackground) {
        painter->fillRect(_drawingRect, QBrush(_color));
    }
}
