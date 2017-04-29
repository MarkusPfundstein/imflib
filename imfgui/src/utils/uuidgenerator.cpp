#include "uuidgenerator.h"

#include <AS_02.h>
#include <sstream>

UUIDGenerator::UUIDGenerator()
{
    //ctor
}

UUIDGenerator::~UUIDGenerator()
{
    //dtor
}

std::string UUIDGenerator::MakeUUID() const
{
    ASDCP::WriterInfo info;
    Kumu::GenRandomUUID(info.AssetUUID);

    char strBuf[41];
    std::stringstream ss;
    ss << ASDCP::UUID(info.AssetUUID).EncodeHex(strBuf, 40);
    return ss.str();
}
