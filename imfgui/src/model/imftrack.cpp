#include "imftrack.h"

IMFTrack::IMFTrack(const std::string& filename, TYPE type)
    :
    IMFPackageItem(filename, type),
    _duration(0),
    _editRate(0, 0)
{
    //ctor
}

IMFTrack::~IMFTrack()
{
    //dtor
}
