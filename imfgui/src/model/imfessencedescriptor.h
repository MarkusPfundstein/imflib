#ifndef IMFESSENCEDESCRIPTOR_H
#define IMFESSENCEDESCRIPTOR_H

#include "genericitem.h"

#include <memory>
#include <string>
#include <list>


class IMFEssenceDescriptor : public GenericItem
{
    public:
    enum TYPE {
        CDCIEssenceDescriptorType,
        RGBAEssenceDescriptorType,
        WaveAudioDescriptorType,
        INVALID_TYPE
    };

    struct MXFEssenceDescriptorBase
    {
        std::string instanceId;
        TYPE type;    

        MXFEssenceDescriptorBase(TYPE t) : instanceId("INVALID"), type(t) {}
        virtual ~MXFEssenceDescriptorBase() {}

        virtual std::string GetTypeAsString() { return "UNDEFINED"; }
    };

    struct RGBAEssenceDescriptor : public MXFEssenceDescriptorBase
    {
        RGBAEssenceDescriptor() : MXFEssenceDescriptorBase(TYPE::RGBAEssenceDescriptorType) {}
        virtual ~RGBAEssenceDescriptor() {}

        virtual std::string GetTypeAsString() { return "RGBADescriptor"; };
    };

    struct CDCIEssenceDescriptor : public MXFEssenceDescriptorBase
    {
        CDCIEssenceDescriptor() : MXFEssenceDescriptorBase(TYPE::CDCIEssenceDescriptorType) {}
        virtual ~CDCIEssenceDescriptor() {}

        virtual std::string GetTypeAsString() { return "CDCIDescriptor"; };
    };

    struct WaveAudioDescriptor : public MXFEssenceDescriptorBase
    {
        WaveAudioDescriptor() : MXFEssenceDescriptorBase(TYPE::WaveAudioDescriptorType) {}
        virtual ~WaveAudioDescriptor() {}

        virtual std::string GetTypeAsString() { return "WAVEPCMDescriptor"; };

    };

    public:
    // TO-DO: this uuid should be used instead of _sourceEncodingUUID in imftrack.h
    IMFEssenceDescriptor(const std::string &uuid, const std::shared_ptr<MXFEssenceDescriptorBase> essenceDescriptor);

    virtual ~IMFEssenceDescriptor();

    const MXFEssenceDescriptorBase& GetEssenceDescriptorData() const 
    { return *_essenceDescriptor; }

    TYPE GetType() const 
    { return _essenceDescriptor->type; }   

    std::string GetTypeAsString() const 
    { return _essenceDescriptor->GetTypeAsString(); };

    private:
    std::shared_ptr<MXFEssenceDescriptorBase> _essenceDescriptor;
};

#endif
