#ifndef CPLResourceRect_H
#define CPLResourceRect_H

#include <memory>
#include <QObject>
#include <QRect>
#include <QColor>
#include <QPainter>

class QEvent;

class CPLResourceRect : public QObject
{
    Q_OBJECT

    public:
        CPLResourceRect(QObject *parent, QPoint s, QPoint e, QColor c);
        virtual ~CPLResourceRect();

        void Draw(QPainter &painter);

        void SetColorRGBA(int r, int g, int b, int a)
        { _color.setRgb(r, g, b, a); }

        virtual bool eventFilter(QObject *object, QEvent *event);

        void SetSelected(bool s)
        { _selected = s; }

    signals:
        void IGotSelected(CPLResourceRect *me);

    protected:
    private:
        // determines if box is selcted
        bool _selected;

        // color in which rect shall be drawn
        QColor _color;

        // int coordinates of sequence rect
        QPoint _startCoordinates;
        QPoint _endCoordinates;

        // box which is drawn
        QRect _sequenceBox;
};

#endif // CPLResourceRect_H
