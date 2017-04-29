#ifndef IMFESSENCEDESCRIPTOR_H
#define IMFESSENCEDESCRIPTOR_H

#include "genericitem.h"

#include <memory>
#include <string>

class IMFEssenceDescriptor : public GenericItem
{
    public:
    enum TYPE {
        CDCIEssenceDescriptor,
        RGBAEssenceDescriptor,
        WaveAudioDescriptor,
        INVALID_TYPE
    };

    struct EssenceDescriptorData {
        TYPE type = TYPE::INVALID_TYPE;
        std::string uuid = "INVALID_UUID";
    };

    public:
    // TO-DO: this uuid should be used instead of _sourceEncodingUUID in imftrack.h
    IMFEssenceDescriptor(const EssenceDescriptorData &d)
        : GenericItem(d.uuid),
        _essenceDescriptorData(d)
    { }

    virtual ~IMFEssenceDescriptor()
    { }

    const EssenceDescriptorData& GetEssenceDescriptorData() const 
    { return _essenceDescriptorData; }

    TYPE GetType() const 
    { return _essenceDescriptorData.type; }   

    std::string GetTypeAsString() const 
    { 
        switch(_essenceDescriptorData.type) {
            case CDCIEssenceDescriptor: return "CDCIDescriptor";
            case WaveAudioDescriptor: return "WAVEPCMDescriptor";
            case RGBAEssenceDescriptor: return "RGBADescriptor";
            default: return "UNKNOWNESSENCETYPE";
        }
    }

    private:
    EssenceDescriptorData _essenceDescriptorData;
};

#endif
