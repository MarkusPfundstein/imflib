#ifndef CPLSEGMENT_H
#define CPLSEGMENT_H

#include "genericitem.h"

class CPLSegment : public GenericItem
{
    public:
        CPLSegment(const std::string &uuid);
        virtual ~CPLSegment();
    protected:
    private:
};

#endif // CPLSEGMENT_H
