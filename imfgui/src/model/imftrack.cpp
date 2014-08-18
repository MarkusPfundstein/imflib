#include "imftrack.h"

IMFTrack::IMFTrack(const std::string &uuid, const std::string& filename, TYPE type)
    :
    IMFPackageItem(uuid, filename, type),
    _bits(0),
    _duration(0),
    _editRate(0, 0),
    _sourceEncodingUUID("")
{
    //ctor
}

IMFTrack::~IMFTrack()
{
    //dtor
}
