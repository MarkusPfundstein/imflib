#include "imfoutputprofile.h"

IMFOutputProfile::IMFOutputProfile(const std::string& filename)
    :
    IMFPackageItem(filename, IMFPackageItem::TYPE::OPL)
{
    //ctor
}

IMFOutputProfile::~IMFOutputProfile()
{
    //dtor
}
