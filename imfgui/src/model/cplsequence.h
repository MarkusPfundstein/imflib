#ifndef CPLSEQUENCE_H
#define CPLSEQUENCE_H

#include "genericitem.h"

class CPLSequence : public GenericItem
{
    public:
        CPLSequence(const std::string &uuid);
        virtual ~CPLSequence();
    protected:
    private:
};

#endif // CPLSEQUENCE_H
