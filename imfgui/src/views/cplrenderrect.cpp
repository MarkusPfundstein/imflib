#include "cplrenderrect.h"

#include <QtGui>
#include <iostream>

CPLRenderRect::CPLRenderRect(QGraphicsItem *_parent)
    :
    QGraphicsItem(_parent),
    _renderBackground(true),
    _drawingRect(0, 0, 0, 0),
    _color(16, 233, 177, 255)
{
    //ctor
    std::cout << "New render rect";
    if (_parent) {
        std::cout << " (with parent) ";
    }
    std::cout << std::endl;
}

CPLRenderRect::~CPLRenderRect()
{
    //dtor
    std::cout << "Delete renderrect" << std::endl;
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
