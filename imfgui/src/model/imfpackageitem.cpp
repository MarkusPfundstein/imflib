#include "imfpackageitem.h"

#include <iostream>

IMFPackageItem::IMFPackageItem(const std::string& filename, TYPE type)
    :
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
