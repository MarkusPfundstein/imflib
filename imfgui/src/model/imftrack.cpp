#include "imftrack.h"

IMFTrack::IMFTrack(const std::string &uuid, const std::string& filename, TYPE type, std::shared_ptr<IMFEssenceDescriptor> essenceDescriptor)
    :
    IMFPackageItem(uuid, filename, type),
    _bits(0),
    _duration(0),
    _editRate(0, 0),
    _essenceDescriptor(essenceDescriptor)
{
    //ctor
}

IMFTrack::~IMFTrack()
{
    //dtor
}
