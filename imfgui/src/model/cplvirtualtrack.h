#ifndef CPLVIRTUALTRACK_H
#define CPLVIRTUALTRACK_H

#include "genericitem.h"
#include <memory>
#include "cplitemlist.h"

class CPLSequence;

class CPLVirtualTrack : public GenericItem, public CPLItemList<CPLSequence>
{
    public:
        CPLVirtualTrack(const std::string &uuid);
        virtual ~CPLVirtualTrack();
};

#endif // CPLVIRTUALTRACK_H
