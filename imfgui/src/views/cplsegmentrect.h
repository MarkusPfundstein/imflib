#ifndef CPLSEGMENTRECT_H
#define CPLSEGMENTRECT_H

#include <QObject>
#include "cplrenderrect.h"
#include "cplpackageitemcontainer.h"

class CPLSegment;
class QGraphicsSceneMouseEvent;

class CPLSegmentRect : public QObject, public CPLRenderRect, public CPLPackageItemContainer<CPLSegment>
{
    Q_OBJECT

    public:
        CPLSegmentRect(QGraphicsItem *parent, const std::shared_ptr<CPLSegment> &segment);
        virtual ~CPLSegmentRect();

        void SetIndex(int i)
        { _index = i; }

        int GetIndex() const
        { return _index; }

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *ev);

     signals:
        void RightMouseClickSignal(QPointF position, CPLSegmentRect &segmentRect);

    protected:
    private:

        // x offset index in drawing
        int _index;
};

#endif // CPLSEGMENTRECT_H
