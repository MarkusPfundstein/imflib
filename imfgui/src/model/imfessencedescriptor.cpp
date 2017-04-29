#include "imfessencedescriptor.h"
#include <iostream>

#include "../utils/uuidgenerator.h"
#include <boost/lexical_cast.hpp>

IMFEssenceDescriptor::IMFEssenceDescriptor(const std::string &uuid, const std::shared_ptr<MXFEssenceDescriptorBase> d)
    : GenericItem(uuid),
    _essenceDescriptor(d)
{ }

IMFEssenceDescriptor::~IMFEssenceDescriptor()
{ }

void IMFEssenceDescriptor::Write(boost::property_tree::ptree &ptParent) const
{
    using namespace boost::property_tree;
    // ID
    std::cout << "XXX ----> write essence descriptor to file: " << GetUUID() << std::endl;
    ptree ptED;
    ptED.put("Id", UUIDStr(GetUUID()));

    ptree essenceRoot;
    essenceRoot.put("r1:SampleRate", "-1/-1");
    

    ptED.add_child("r0:" + GetTypeAsString(), essenceRoot);

    ptParent.add_child("EssenceDescriptor", ptED);
}


