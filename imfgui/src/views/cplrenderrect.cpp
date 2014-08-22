#include "cplrenderrect.h"

#include <QtGui>
#include <iostream>

static int DEBUG_COUNT = 0;

CPLRenderRect::CPLRenderRect(QGraphicsItem *_parent)
    :
    QGraphicsRectItem(_parent),
    _renderBackground(true),
    _color(16, 233, 177, 255),
    _drawAttachAreaLeft(false),
    _drawAttachAreaRight(false),
    _acceptAttachment(false)
{
    DEBUG_COUNT++;
}

CPLRenderRect::~CPLRenderRect()
{
    //dtor
    std::cout << "Delete renderrect [" << --DEBUG_COUNT << " left]" << std::endl;
}

void CPLRenderRect::SetDrawingRect(const QRect &r)
{
    setRect(r);
}

QRect CPLRenderRect::RightIntersectionRect() const
{
    float intersectWidth = 0.75f;
    float startX = intersectWidth * rect().width();
    float startY = roundf(y());
    float endX = (1 - intersectWidth) * rect().width();
    float endY = rect().height();

    return QRect(startX, startY, endX, endY);
}

QRect CPLRenderRect::LeftIntersectionRect() const
{
    float intersectWidth = 0.75f;
    float startX = 0;
    float startY = roundf(y());
    float endX = (1 - intersectWidth) * rect().width();
    float endY = rect().height();

    return QRect(startX, startY, endX, endY);
}

void CPLRenderRect::paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget)
{
    if (_renderBackground) {
        setBrush(_color);
        QGraphicsRectItem::paint(painter, item, widget);
        if (_acceptAttachment && _drawAttachAreaRight) {
            painter->fillRect(RightIntersectionRect(), QBrush(QColor(0, 0, 0, 130)));
        }
        if (_acceptAttachment && _drawAttachAreaLeft) {
            painter->fillRect(LeftIntersectionRect(), QBrush(QColor(0, 0, 0, 130)));
        }
    }
}
