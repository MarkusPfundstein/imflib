#include "imfoutputprofile.h"

IMFOutputProfile::IMFOutputProfile(const std::string &uuid, const std::string& filename)
    :
    IMFPackageItem(uuid, filename, IMFPackageItem::TYPE::OPL)
{
    //ctor
}

IMFOutputProfile::~IMFOutputProfile()
{
    //dtor
}
