#include "imfpackageitem.h"

#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>

IMFPackageItem::IMFPackageItem(const std::string &uuid, const std::string& filename, TYPE type)
    :
    GenericItem(uuid),
    _filename(filename),
    _type(type)
{
    //ctor
}

IMFPackageItem::~IMFPackageItem()
{
    //dtor
    std::cout << "delete package item" << std::endl;
}

std::string IMFPackageItem::GetFileName() const
{
    using namespace boost::filesystem;

    return basename(_filename) + ((_type == VIDEO || _type == AUDIO) ? ".mxf" : ".xml");
}

int IMFPackageItem::GetFileSize() const
{
    using namespace boost::filesystem;

    return file_size(_filename);
}

std::string IMFPackageItem::TypeString() const
{
    switch (_type) {
        case VIDEO: return "Video";
        case AUDIO: return "Audio";
        case CPL: return "CPL";
        case OPL: return "OPL";
        default: return "ERR_TYPE";
    }
}
