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
    
    // TO-DO: Make this all more based on MXF structure. I omit a few details here
    // fields in this structs have same name as they should have in XML. besides not beginning with capital
    // Note: some files have different name in asdcplib. I've put them in comments 
    // The very base class. Everything EssenceDescritpor derives from that
    struct TransferFileBase
    {
        std::string instanceId;

        std::list<std::shared_ptr<TransferFileBase>> subDescriptors;

        TransferFileBase() : instanceId("INVALID") {}
        virtual ~TransferFileBase() {}

        virtual std::string GetTypeAsString() const { return "UNDEFINED"; }

        virtual void Write(boost::property_tree::ptree &) const;
    };
 
    // Video and Audio Descriptors derive from this one. TimedText: Idk
    struct MXFEssenceDescriptorBase : public TransferFileBase
    {
        TYPE type;    
        RationalNumber sampleRate;
        uint64_t essenceLength;      // duration, asdcplib: ContainerDuration
        std::string containerFormat; // urn:smpte:ul, asdcplib: EssenceContainer
        uint32_t linkedTrackId;

        MXFEssenceDescriptorBase(TYPE t) : TransferFileBase(), type(t) {}
        virtual ~MXFEssenceDescriptorBase() {}

        virtual void Write(boost::property_tree::ptree &) const;
    };

    /* AUDIO DESCRIPTORS */
    struct AudioEssenceDescriptor : public MXFEssenceDescriptorBase
    {   
        uint8_t locked;
        std::string soundCompression;   // urn:smpte:ul, asdcplib: SoundEssenceCoding
        RationalNumber audioSampleRate; // == SampleRate, asdcplib: AudioSamplingRate
        uint32_t channelCount;
        uint32_t quantizationBits;

        AudioEssenceDescriptor(TYPE t) : MXFEssenceDescriptorBase(t) {}
        virtual ~AudioEssenceDescriptor() {}

        virtual void Write(boost::property_tree::ptree &) const;
    };

    struct WaveAudioDescriptor : public AudioEssenceDescriptor
    {
        uint16_t blockAlign;
        uint32_t averageBytesPerSecond;
        std::string channelAssignment;  // urn:smpte:ul

        WaveAudioDescriptor() : AudioEssenceDescriptor(TYPE::WaveAudioDescriptorType) {} virtual ~WaveAudioDescriptor() {} 
        virtual std::string GetTypeAsString() const { return "WAVEPCMDescriptor"; };
        
        virtual void Write(boost::property_tree::ptree &) const;
    };
    /* END AUDIO DESCRIPTORS */

    /* VIDEO DESCRIPTORS */
    struct VideoEssenceDescriptor : public MXFEssenceDescriptorBase
    {
        std::string transferCharacteristic; // urn:smpte:ul
        std::string codingEquations;        // urn:smpte:ul
        std::string colorPrimaries;         // urn:smpte:ul
        uint8_t frameLayout;            
        std::string pictureCompression;     // urn:smpte:ul, in asdcplib called PictureEssenceCoding

        RationalNumber imageAspectRatio;
        uint32_t storedWidth;
        uint32_t storedHeight;

        VideoEssenceDescriptor(TYPE t) : MXFEssenceDescriptorBase(t) {}
        virtual ~VideoEssenceDescriptor() {}

        virtual void Write(boost::property_tree::ptree &) const;
    };

    struct RGBAEssenceDescriptor : public VideoEssenceDescriptor
    {
        uint32_t componentMaxRef;
        uint32_t componentMinRef;

        RGBAEssenceDescriptor() : VideoEssenceDescriptor(TYPE::RGBAEssenceDescriptorType) {}
        virtual ~RGBAEssenceDescriptor() {}

        virtual std::string GetTypeAsString() const { return "RGBADescriptor"; };
    
        virtual void Write(boost::property_tree::ptree &) const;
    };

    struct CDCIEssenceDescriptor : public VideoEssenceDescriptor
    {
        CDCIEssenceDescriptor() : VideoEssenceDescriptor(TYPE::CDCIEssenceDescriptorType) {}
        virtual ~CDCIEssenceDescriptor() {}

        virtual std::string GetTypeAsString() const { return "CDCIDescriptor"; };
    
        virtual void Write(boost::property_tree::ptree &) const;
    };
    /* END VIDEO DESCRIPTORS */
    
    // undefined zone ... don't go there

    /* START AUDIO SUB DESCRIPTORS */
    struct MCALabelSubDescriptor : public TransferFileBase
    {
        std::string mcaLinkId;              // UUID 
        std::string mcaLabelDictionaryId;   // UL
        std::string mcaTagName;             // string
        std::string mcaTagSymbol;           // string
        std::string rfc5646SpokenLanguage;         // locale
        // TO-DO: check if mcaAudioElementKind, mcaAudioContentKind, mcaTitle & mcaVersion should go here as well. Right now they are in SoundfieldGroupLabelSubDescriptor

        MCALabelSubDescriptor() : TransferFileBase() {};
        virtual ~MCALabelSubDescriptor() {};

        virtual void Write(boost::property_tree::ptree &) const;
    };

    struct AudioChannelLabelSubDescriptor : public MCALabelSubDescriptor
    {
        std::string soundFieldGroupLinkId;  // UUID
        uint16_t mcaChannelId;

        AudioChannelLabelSubDescriptor() : MCALabelSubDescriptor() {};
        virtual ~AudioChannelLabelSubDescriptor() {};

        virtual void Write(boost::property_tree::ptree &) const;
    
        virtual std::string GetTypeAsString() const { return "AudioChannelLabelSubDescriptor"; };
    };

    struct SoundfieldGroupLabelSubDescriptor : public MCALabelSubDescriptor
    {
        // TO-DO: check if the following four should go into MCALabelSubDescriptor. Need to fix asdcplib then I guess
        std::string mcaAudioElementKind;    // string
        std::string mcaAudioContentKind;    // string
        std::string mcaTitle;               // string
        std::string mcaTitleVersion;        // string

        SoundfieldGroupLabelSubDescriptor() : MCALabelSubDescriptor() {};
        virtual ~SoundfieldGroupLabelSubDescriptor() {}
        
        virtual void Write(boost::property_tree::ptree &) const;
        
        virtual std::string GetTypeAsString() const { return "SoundfieldGroupLabelSubDescriptor"; };
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

    /*
    std::string GetTypeAsString() const 
    { return _essenceDescriptor->GetTypeAsString(); };
    */

    private:
    std::shared_ptr<MXFEssenceDescriptorBase> _essenceDescriptor;


};

#endif
