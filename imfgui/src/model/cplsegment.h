#ifndef CPLSEGMENT_H
#define CPLSEGMENT_H

#include "genericitem.h"
#include "cplitemlist.h"

#include <memory>

class CPLSequence;

class CPLSegment : public GenericItem, public CPLItemList<CPLSequence>
{
    public:
        CPLSegment(const std::string &uuid);
        virtual ~CPLSegment();

        int GetDuration() const;
};

#endif // CPLSEGMENT_H
