#include "cplsequence.h"

#include "cplresource.h"

CPLSequence::CPLSequence(const std::string &uuid)
    :
    GenericItem(uuid),
    _resources()
{
    //ctor
}

CPLSequence::~CPLSequence()
{
    //dtor
}

void CPLSequence::AddResource(const std::shared_ptr<CPLResource> &resource)
{
    _resources.push_back(resource);
}
