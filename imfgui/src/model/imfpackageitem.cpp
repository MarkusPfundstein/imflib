#include "imfpackageitem.h"

#include <iostream>

IMFPackageItem::IMFPackageItem(const std::string& filename)
    :
    _filename(filename)
{
    //ctor
}

IMFPackageItem::~IMFPackageItem()
{
    //dtor
    std::cout << "delete package item" << std::endl;
}
