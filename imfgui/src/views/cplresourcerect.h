#ifndef CPLResourceRect_H
#define CPLResourceRect_H

#include <QGraphicsItem>
#include <QRect>
#include <QColor>

class CPLResourceRect : public QGraphicsItem
{
    public:
        CPLResourceRect(const QRect &r, const QColor &fillColor, const QColor &shadowColor);
        virtual ~CPLResourceRect();

        QRectF boundingRect() const;

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget);

        void SetShadowOffsets(int ox, int oy)
        { _shadowOffsetX = ox; _shadowOffsetY = oy; }

    private:
        QRect _drawingRect;

        QColor _drawingRectFillColor;
        QColor _drawingRectBorderColor;

        int _shadowOffsetX;
        int _shadowOffsetY;
};

#endif // CPLResourceRect_H
