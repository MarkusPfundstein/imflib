#ifndef CPLVIRTUALTRACK_H
#define CPLVIRTUALTRACK_H

#include "genericitem.h"

class CPLVirtualTrack : public GenericItem
{
    public:
        CPLVirtualTrack(const std::string &uuid);
        virtual ~CPLVirtualTrack();
    protected:
    private:
};

#endif // CPLVIRTUALTRACK_H
