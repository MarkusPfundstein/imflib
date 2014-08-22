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

        void SetDrawAttachAreaRight(bool r)
        { _drawAttachAreaRight = _acceptAttachment && r; }

        void SetDrawAttachAreaLeft(bool r)
        { _drawAttachAreaLeft = _acceptAttachment && r; }

        bool GetDrawAttachAreaRight() const
        { return _acceptAttachment && _drawAttachAreaRight; }

        bool GetDrawAttachAreaLeft() const
        { return _acceptAttachment && _drawAttachAreaLeft; }

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
