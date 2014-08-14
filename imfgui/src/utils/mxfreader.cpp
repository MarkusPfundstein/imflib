#include "mxfreader.h"

#include <AS_02.h>

MXFReader::MXFReader(const std::string& filename)
    : _filename(filename)
{
    //ctor
}

MXFReader::~MXFReader()
{
    //dtor
}

MXFReader::ESSENCE_TYPE MXFReader::GetEssenceType() const
{
    ASDCP::EssenceType_t EssenceType;
    ASDCP::Result_t result = ASDCP::EssenceType(_filename, EssenceType);

    if (ASDCP_SUCCESS(result)) {
        switch (EssenceType) {
            case ASDCP::ESS_AS02_JPEG_2000:
                return ESSENCE_TYPE::VIDEO;
                break;

            case ASDCP::ESS_AS02_PCM_24b_48k:
            case ASDCP::ESS_AS02_PCM_24b_96k:
                return ESSENCE_TYPE::AUDIO;
                break;

            default:
                break;
        }
    }
    return ESSENCE_TYPE::INVALID;
}
