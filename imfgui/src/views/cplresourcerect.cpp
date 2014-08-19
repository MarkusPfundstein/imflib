#include "cplresourcerect.h"

#include <QEvent>
#include <QMouseEvent>
#include <iostream>
#include <QtGui>

static int DEBUG_COUNT = 0;

CPLResourceRect::CPLResourceRect(const QRect &r, const QColor& fillColor, const QColor &shadowColor)
    :
    QGraphicsItem(),
    _drawingRect(r),
    _drawingRectFillColor(fillColor),
    _drawingRectBorderColor(shadowColor),
    _shadowOffsetX(1),
    _shadowOffsetY(1)
{
    DEBUG_COUNT++;
    std::cout << "Make resource rect [" << DEBUG_COUNT << "]" << std::endl;
}

CPLResourceRect::~CPLResourceRect()
{
    std::cout << "delete resource rect [" << DEBUG_COUNT-- << " left]" << std::endl;
}

QRectF CPLResourceRect::boundingRect() const
{
    return _drawingRect;
}

void CPLResourceRect::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*item*/, QWidget* /*widget*/)
{
    QBrush fillBrush(_drawingRectFillColor);
    QBrush shadowBrush(_drawingRectBorderColor);

    QRect shadowRect(_drawingRect);
    shadowRect.setX(shadowRect.x() + _shadowOffsetX);
    shadowRect.setY(shadowRect.y() + _shadowOffsetY);
    shadowRect.setHeight(shadowRect.height() + _shadowOffsetX);
    shadowRect.setWidth(shadowRect.width() + _shadowOffsetY);
    painter->fillRect(shadowRect, shadowBrush);
    painter->fillRect(_drawingRect, fillBrush);
    //
}
