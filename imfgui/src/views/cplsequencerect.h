#ifndef CPLSEQUENCERECT_H
#define CPLSEQUENCERECT_H

#include "cplrenderrect.h"
#include <memory>

class CPLResourceRect;
class CPLSequence;

class CPLSequenceRect : public CPLRenderRect
{
    public:
        CPLSequenceRect(QGraphicsItem *parent, const std::shared_ptr<CPLSequence> &sequence);
        virtual ~CPLSequenceRect();

        void SetLastItem(CPLResourceRect* l)
        { _lastItem = l; }

        CPLResourceRect* GetLastItem() const
        { return _lastItem; }

        void SetFirstItem(CPLResourceRect* f)
        { _firstItem = f; }

        CPLResourceRect* GetFirstItem() const
        { return _firstItem; }

        void SetTrackIndex(int t)
        { _trackIndex = t; }

        int GetTrackIndex() const
        { return _trackIndex; }

        const std::shared_ptr<CPLSequence>& GetSequence() const
        { return _sequence; }

    protected:
    private:

        std::shared_ptr<CPLSequence> _sequence;

        // trackIdx
        int _trackIndex;

        // pointer to last item
        CPLResourceRect *_lastItem;

        // pointer to first item
        CPLResourceRect *_firstItem;
};

#endif // CPLSEQUENCERECT_H
