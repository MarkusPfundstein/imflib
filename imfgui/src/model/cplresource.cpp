#include "cplresource.h"

CPLResource::CPLResource(const std::string &uuid, const std::shared_ptr<IMFTrack> &track)
    :
    GenericItem(uuid),
    _track(track),
    _repeatCount(0),
    _entryPoint(0),
    _sourceDuration(0),
    _sourceEncoding(""),
    _keyId(""),
    _hash("")
{
    //ctor
}

CPLResource::~CPLResource()
{
    //dtor
}
