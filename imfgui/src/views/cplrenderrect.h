#ifndef CPLRENDERRECT_H
#define CPLRENDERRECT_H

#include <QGraphicsRectItem>

class CPLRenderRect : public QGraphicsRectItem
{
    public:
        CPLRenderRect(QGraphicsItem *parent = nullptr);
        virtual ~CPLRenderRect();

        // draws _drawingRect in _color
        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget);

        void SetDrawingRect(const QRect &r);

        void SetColor(const QColor &c)
        { _color = c; }

        void SetRenderBackground(bool b)
        { _renderBackground = b; }

        void SetDrawAttachAreaRight(bool r) { _drawAttachAreaRight = r; }
        void SetDrawAttachAreaLeft(bool r) { _drawAttachAreaLeft = r; }

        bool GetDrawAttachAreaRight() const
        { return _drawAttachAreaRight; }

        bool GetDrawAttachAreaLeft() const
        { return _drawAttachAreaLeft; }

        virtual QRect RightIntersectionRect() const;
        virtual QRect LeftIntersectionRect() const;

        void SetAcceptAttachment(bool b)
        { _acceptAttachment = b; }

    protected:
        // render background
        bool _renderBackground;

        // color of segment
        QColor _color;

        bool _drawAttachAreaLeft;
        bool _drawAttachAreaRight;

        bool _acceptAttachment;
};

#endif // CPLRENDERRECT_H
