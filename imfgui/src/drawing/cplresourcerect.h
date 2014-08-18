#ifndef CPLResourceRect_H
#define CPLResourceRect_H

#include <memory>
#include <QRect>
#include <QColor>
#include <QPainter>

class CPLResourceRect
{
    public:
        CPLResourceRect(int x1, int y1, int x2, int y2);
        CPLResourceRect(int x1, int y1, int x2, int y2, int r, int g, int b, int a);
        CPLResourceRect(QPoint s, QPoint e, QColor c);
        virtual ~CPLResourceRect();

        void Draw(QPainter &painter);

        void SetColorRGBA(int r, int g, int b, int a)
        { _color.setRgb(r, g, b, a); }

    protected:
    private:

        // color in which rect shall be drawn
        QColor _color;

        // int coordinates of sequence rect
        QPoint _startCoordinates;
        QPoint _endCoordinates;

        // box which is drawn
        QRect _sequenceBox;
};

#endif // CPLResourceRect_H
