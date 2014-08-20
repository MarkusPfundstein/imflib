#ifndef CPLSEQUENCERECT_H
#define CPLSEQUENCERECT_H

#include "cplrenderrect.h"

class CPLResourceRect;

class CPLSequenceRect : public CPLRenderRect
{
    public:
        CPLSequenceRect(QGraphicsItem *parent = nullptr);
        virtual ~CPLSequenceRect();

        void SetLastItem(CPLResourceRect* l)
        { _lastItem = l; }

        CPLResourceRect* GetLastItem() const
        { return _lastItem; }

        int SetTrackIndex(int t)
        { _trackIndex = t; }

        int GetTrackIndex() const
        { return _trackIndex; }

    protected:
    private:

        // trackIdx
        int _trackIndex;

        // pointer to last item
        CPLResourceRect *_lastItem;
};

#endif // CPLSEQUENCERECT_H
