#include "cplresourcerect.h"

#include "../model/cplresource.h"
#include "../model/imftrack.h"

#include <QEvent>
#include <QMouseEvent>
#include <iostream>
#include <QtGui>

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
    _shadowOffsetY(1)
{
    setAcceptHoverEvents(true);
    SetDrawingRect(r);
    SetColor(fillColor);

    DEBUG_COUNT++;
    //std::cout << "Make resource rect [" << DEBUG_COUNT << "]" << std::endl;
}

CPLResourceRect::~CPLResourceRect()
{
    std::cout << "delete resource rect [" << --DEBUG_COUNT << " left]" << std::endl;
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

void CPLResourceRect::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    std::cout << "Custom item clicked." << std::endl;
    if (ev->button() == Qt::RightButton) {
        ev->accept();
        emit RightMouseClickSignal(QPoint(roundf(ev->pos().x()), roundf(ev->pos().y())), *this);
    }
    //update();
}

void CPLResourceRect::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*item*/, QWidget* /*widget*/)
{
    QBrush fillBrush(_color);
    QBrush shadowBrush(_drawingRectBorderColor);

    QRect shadowRect(_drawingRect);
    shadowRect.setX(shadowRect.x() + _shadowOffsetX);
    shadowRect.setY(shadowRect.y() + _shadowOffsetY);
    shadowRect.setHeight(shadowRect.height() + _shadowOffsetX);
    shadowRect.setWidth(shadowRect.width() + _shadowOffsetY);
    painter->fillRect(shadowRect, shadowBrush);

    QRect imageRect;
    QRect textRect;

    int imageWidth = roundf(_identifierItem.width() * (_drawingRect.height() / (float)_identifierItem.height()));

    if (_mouseIsOver == false) {
        painter->fillRect(_drawingRect, fillBrush);
        imageRect = _drawingRect;
        textRect = _drawingRect;
    } else {
        imageRect = shadowRect;
        textRect = shadowRect;
    }

    imageRect.setLeft(imageRect.left() + 3);
    imageRect.setWidth(imageWidth);
    textRect.setLeft(imageRect.right() + 10);

    painter->drawImage(imageRect, _identifierItem);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, QString::fromStdString(_resource->GetTrack()->GetFileName()));
}

#include "../moc_cplresourcerect.cpp"
