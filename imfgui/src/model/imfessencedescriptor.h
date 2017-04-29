#ifndef IMFESSENCEDESCRIPTOR_H
#define IMFESSENCEDESCRIPTOR_H

#include "genericitem.h"
#include "../utils/common.h"

#include <memory>
#include <string>
#include <list>
#include <tuple>

#include <boost/property_tree/ptree.hpp>

class IMFEssenceDescriptor : public GenericItem
{
    public:

    enum TYPE {
        CDCIEssenceDescriptorType,
        RGBAEssenceDescriptorType,
        WaveAudioDescriptorType,
        INVALID_TYPE
    };
    
    // fields in this structs have same name as they should have in XML. besides not beginning with capital
    // Note: some files have different name in asdcplib. I've put them in comments
    struct MXFEssenceDescriptorBase
    {
        std::string instanceId;
        TYPE type;    
        RationalNumber sampleRate;
        uint64_t essenceLength;      // duration, asdcplib: ContainerDuration
        std::string containerFormat; // urn:smpte:ul, asdcplib: EssenceContainer
        uint32_t linkedTrackId;

        MXFEssenceDescriptorBase(TYPE t) : type(t) {}
        virtual ~MXFEssenceDescriptorBase() {}

        virtual std::string GetTypeAsString() { return "UNDEFINED"; }
    };

    struct AudioEssenceDescriptor : public MXFEssenceDescriptorBase
    {   
        uint8_t locked;
        std::string soundCompression;   // urn:smpte:ul, asdcplib: SoundEssenceCoding
        RationalNumber audioSampleRate; // == SampleRate, asdcplib: AudioSamplingRate
        uint32_t channelCount;
        uint32_t quantizationBits;

        AudioEssenceDescriptor(TYPE t) : MXFEssenceDescriptorBase(t) {}
        virtual ~AudioEssenceDescriptor() {}
    };

    struct WaveAudioDescriptor : public AudioEssenceDescriptor
    {
        uint16_t blockAlign;
        uint32_t averageBytesPerSecond;
        std::string channelAssignment;  // urn:smpte:ul

        WaveAudioDescriptor() : AudioEssenceDescriptor(TYPE::WaveAudioDescriptorType) {} virtual ~WaveAudioDescriptor() {} 
        virtual std::string GetTypeAsString() { return "WAVEPCMDescriptor"; };

    };

    struct VideoEssenceDescriptor : public MXFEssenceDescriptorBase
    {
        std::string transferCharacteristic; // urn:smpte:ul
        std::string codingEquations;        // urn:smpte:ul
        std::string colorPrimaries;         // urn:smpte:ul
        std::string frameLayout;            // urn:smpte:ul
        std::string pictureCompression;     // urn:smpte:ul, in asdcplib called PictureEssenceCoding

        RationalNumber imageAspectRatio;
        uint32_t storedWidth;
        uint32_t storedHeight;

        VideoEssenceDescriptor(TYPE t) : MXFEssenceDescriptorBase(t) {}
        virtual ~VideoEssenceDescriptor() {}
    };

    struct RGBAEssenceDescriptor : public VideoEssenceDescriptor
    {
        uint32_t componentMaxRef;
        uint32_t componentMinRef;

        RGBAEssenceDescriptor() : VideoEssenceDescriptor(TYPE::RGBAEssenceDescriptorType) {}
        virtual ~RGBAEssenceDescriptor() {}

        virtual std::string GetTypeAsString() { return "RGBADescriptor"; };
    };

    struct CDCIEssenceDescriptor : public VideoEssenceDescriptor
    {
        CDCIEssenceDescriptor() : VideoEssenceDescriptor(TYPE::CDCIEssenceDescriptorType) {}
        virtual ~CDCIEssenceDescriptor() {}

        virtual std::string GetTypeAsString() { return "CDCIDescriptor"; };
    };

    public:
    // TO-DO: this uuid should be used instead of _sourceEncodingUUID in imftrack.h
    IMFEssenceDescriptor(const std::string &uuid, const std::shared_ptr<MXFEssenceDescriptorBase> essenceDescriptor);

    virtual ~IMFEssenceDescriptor();

    void Write(boost::property_tree::ptree &ptParent) const;

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
