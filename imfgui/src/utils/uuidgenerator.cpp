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

std::string ULStr(const std::string &ul) {
    return std::string("urn:smpte:ul:") + ul;
}

std::string UUIDStr(const std::string &uuid) {
    return std::string("urn:uuid:") + uuid;
}

void UUIDClean(std::string &s) {
    s.erase(0, std::string("urn:uuid:").length());  // delete urn:uuid:
}
