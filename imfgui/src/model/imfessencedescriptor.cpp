#include "imfessencedescriptor.h"
#include <AS_02.h>

IMFEssenceDescriptor::IMFEssenceDescriptor(const std::string &uuid, const std::shared_ptr<MXFEssenceDescriptorBase> d)
    : GenericItem(uuid),
    _essenceDescriptor(d)
{ }

IMFEssenceDescriptor::~IMFEssenceDescriptor()
{ }
