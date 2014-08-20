#ifndef CPLSEGMENTRECT_H
#define CPLSEGMENTRECT_H

#include "cplrenderrect.h"

class QGraphicsSceneMouseEvent;

class CPLSegmentRect : public CPLRenderRect
{
    public:
        CPLSegmentRect(QGraphicsItem *parent = nullptr);
        virtual ~CPLSegmentRect();

        void SetIndex(int i)
        { _index = i; }

        int GetIndex() const
        { return _index; }

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *ev);

    protected:
    private:

        // x offset index in drawing
        int _index;
};

#endif // CPLSEGMENTRECT_H
