#ifndef CPLRENDERRECT_H
#define CPLRENDERRECT_H

#include <QGraphicsItem>

class CPLRenderRect : public QGraphicsItem
{
    public:
        CPLRenderRect(QGraphicsItem *parent = nullptr);
        virtual ~CPLRenderRect();

        // returns _drawingRect
        virtual QRectF boundingRect() const;

        // draws _drawingRect in _color
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget);

        void SetDrawingRect(const QRect &r);

        void SetColor(const QColor &c)
        { _color = c; }

    protected:

        // position
        QRect _drawingRect;

        // color of segment
        QColor _color;
};

#endif // CPLRENDERRECT_H
