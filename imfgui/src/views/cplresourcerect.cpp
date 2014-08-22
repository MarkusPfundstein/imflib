#include "cplresourcerect.h"

#include "../model/cplresource.h"
#include "../model/imftrack.h"

#include <QEvent>
#include <QMouseEvent>
#include <iostream>
#include <QtGui>
#include <cmath>

static int DEBUG_COUNT = 0;

CPLResourceRect::CPLResourceRect(QGraphicsItem *_parent,
                                 const std::shared_ptr<CPLResource> &resource,
                                 const QRect &r,
                                 const QColor& fillColor,
                                 const QColor &shadowColor,
                                 const QImage &identifierIcon)
    :
    CPLRenderRect(_parent),
    _drawingRectBorderColor(shadowColor),
    _identifierItem(identifierIcon),
    _resource(resource),
    _mouseIsOver(false),
    _shadowOffsetX(1),
    _shadowOffsetY(1),
    _verticalOffset(3)
{
    setAcceptHoverEvents(true);
    SetDrawingRect(r);
    SetColor(fillColor);
    SetAcceptAttachment(true);

    DEBUG_COUNT++;
    //std::cout << "Make resource rect [" << DEBUG_COUNT << "]" << std::endl;
}

CPLResourceRect::~CPLResourceRect()
{
    std::cout << "delete resource rect [" << --DEBUG_COUNT << " left]" << std::endl;
}

QRect CPLResourceRect::RightIntersectionRect() const
{
    float intersectWidth = 0.75f;
    float startX = intersectWidth * rect().width();
    float startY = roundf(y()) - _verticalOffset;
    float endX = (1 - intersectWidth) * rect().width();
    float endY = rect().height();

    return QRect(startX, startY, endX, endY);
}

QRect CPLResourceRect::LeftIntersectionRect() const
{
    float intersectWidth = 0.75f;
    float startX = 0;
    float startY = roundf(y()) - _verticalOffset;
    float endX = (1 - intersectWidth) * rect().width();
    float endY = rect().height();

    return QRect(startX, startY, endX, endY);
}

void CPLResourceRect::hoverEnterEvent(QGraphicsSceneHoverEvent* ev)
{
    _mouseIsOver = true;
    update();
}

void CPLResourceRect::hoverLeaveEvent(QGraphicsSceneHoverEvent* ev)
{
    _mouseIsOver = false;
    update();
}

void CPLResourceRect::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    //_mouseIsOver = false;
}

void CPLResourceRect::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    std::cout << "ResourceRect pressed" << std::endl;
    if (ev->button() == Qt::RightButton) {
        ev->accept();
        emit RightMouseClickSignal(ev->screenPos(), *this);
    }
}

void CPLResourceRect::setPos(const QPointF &pos)
{
    CPLRenderRect::setPos(pos.x(), pos.y() + _verticalOffset);
}

void CPLResourceRect::setPos(qreal x, qreal y)
{
    CPLRenderRect::setPos(x, y + _verticalOffset);
}

void CPLResourceRect::paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget)
{
    QBrush fillBrush(_color);
    QBrush shadowBrush(_drawingRectBorderColor);

    QRectF shadowRect(rect());
    shadowRect.setX(shadowRect.x() + _shadowOffsetX);
    shadowRect.setY(shadowRect.y() + _shadowOffsetY);
    shadowRect.setHeight(shadowRect.height() + _shadowOffsetX);
    shadowRect.setWidth(shadowRect.width() + _shadowOffsetY);
    painter->fillRect(shadowRect, shadowBrush);

    QRectF imageRect;
    QRectF textRect;

    int imageWidth = roundf(_identifierItem.width() * (rect().height() / (float)_identifierItem.height()));

    if (_mouseIsOver == false) {
        setBrush(fillBrush);
        painter->fillRect(rect(), fillBrush);
        imageRect = rect();
        textRect = rect();
    } else {
        imageRect = shadowRect;
        textRect = shadowRect;
    }

    imageRect.setLeft(imageRect.left() + 3);
    imageRect.setWidth(imageWidth);
    textRect.setLeft(imageRect.right() + 10);

    painter->drawImage(imageRect, _identifierItem);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, QString::fromStdString(_resource->GetTrack()->GetFileName()));

    if (_drawAttachAreaRight) {
        painter->fillRect(RightIntersectionRect(), QBrush(QColor(0, 0, 0, 130)));
    }
    if (_drawAttachAreaLeft) {
        painter->fillRect(LeftIntersectionRect(), QBrush(QColor(0, 0, 0, 130)));
    }
}


#include "../moc_cplresourcerect.cpp"
