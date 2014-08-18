#ifndef CPLRESOURCE_H
#define CPLRESOURCE_H

#include "genericitem.h"

class CPLResource : public GenericItem
{
    public:
        CPLResource(const std::string &uuid);
        virtual ~CPLResource();
    protected:
    private:
};

#endif // CPLRESOURCE_H
