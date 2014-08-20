#ifndef CPLResourceRect_H
#define CPLResourceRect_H

#include "cplrenderrect.h"
#include <QRect>
#include <QColor>
#include <memory>

class CPLResource;
class QGraphicsScenceMouseEvent;

class CPLResourceRect : public CPLRenderRect
{
    public:
        CPLResourceRect(QGraphicsItem *parent,
                        const std::shared_ptr<CPLResource> &resource,
                        const QRect &r,
                        const QColor &fillColor,
                        const QColor &shadowColor,
                        const QImage &identifierIcon);
        virtual ~CPLResourceRect();

        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget);

        void SetShadowOffsets(int ox, int oy)
        { _shadowOffsetX = ox; _shadowOffsetY = oy; }

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *ev);
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *ev);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *ev);

    private:
        QColor _drawingRectBorderColor;

        const QImage& _identifierItem;
        std::shared_ptr<CPLResource> _resource;

        bool _mouseIsOver;

        int _shadowOffsetX;
        int _shadowOffsetY;
};

#endif // CPLResourceRect_H
