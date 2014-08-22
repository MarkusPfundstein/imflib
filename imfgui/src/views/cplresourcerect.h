#ifndef CPLResourceRect_H
#define CPLResourceRect_H

#include "cplrenderrect.h"
#include <QRect>
#include <QColor>
#include <memory>

class CPLResource;
class QGraphicsScenceMouseEvent;

class CPLResourceRect : public QObject, public CPLRenderRect
{
    Q_OBJECT

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

        const std::shared_ptr<CPLResource>& GetResource() const
        { return _resource; }

        void setPos(const QPointF &pos);
        void setPos(qreal x, qreal y);

        QRect RightIntersectionRect() const;
        QRect LeftIntersectionRect() const;

        void mousePressEvent(QGraphicsSceneMouseEvent *ev);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *ev);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *ev);

    signals:
        void RightMouseClickSignal(QPoint position, CPLResourceRect &resourceRect);

    private:
        QColor _drawingRectBorderColor;

        const QImage& _identifierItem;
        std::shared_ptr<CPLResource> _resource;

        bool _mouseIsOver;

        int _shadowOffsetX;
        int _shadowOffsetY;

        int _verticalOffset;
};

#endif // CPLResourceRect_H
