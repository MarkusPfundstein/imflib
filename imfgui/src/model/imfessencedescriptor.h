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

        //uint8_t fieldDominance;

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
        std::string pixelLayout;    // raw string, interpret and parse when writing

        RGBAEssenceDescriptor() : VideoEssenceDescriptor(TYPE::RGBAEssenceDescriptorType) {}
        virtual ~RGBAEssenceDescriptor() {}

        virtual std::string GetTypeAsString() const { return "RGBADescriptor"; };
    
        virtual void Write(boost::property_tree::ptree &) const;
    };

    struct CDCIEssenceDescriptor : public VideoEssenceDescriptor
    {
        uint8_t horizontalSubsampling;
        uint8_t verticalSubsampling;
        uint16_t componentDepth;
        uint32_t whiteRefLevel;
        uint32_t blackRefLevel;

        CDCIEssenceDescriptor() : VideoEssenceDescriptor(TYPE::CDCIEssenceDescriptorType) {}
        virtual ~CDCIEssenceDescriptor() {}

        virtual std::string GetTypeAsString() const { return "CDCIDescriptor"; };
    
        virtual void Write(boost::property_tree::ptree &) const;
    };
    /* END VIDEO DESCRIPTORS */

    /* START VIDEO SUB DESCRIPTORS */
    // base class for JPEG2000 SubDescriptors
    struct JPEG2000SubDescriptor : public TransferFileBase
    {
        uint16_t rsize;
        uint32_t xsize;
        uint32_t ysize;
        uint32_t xOsize;
        uint32_t yOsize;
        uint32_t xTsize;
        uint32_t yTsize;
        uint32_t xTOsize;
        uint32_t yTOsize;
        uint16_t csize;

        std::string pictureComponentSizing;         // raw string -> parsed and expanded on write
        std::string codingStyleDefault;             // raw string
        std::string quantizationDefault;            // raw string
        std::string j2cLayout;                      // raw string -> parsed and expanded on write

        JPEG2000SubDescriptor() : TransferFileBase() {}
        virtual ~JPEG2000SubDescriptor() {};

        virtual std::string GetTypeAsString() const { return "JPEG2000SubDescriptor"; };

        virtual void Write(boost::property_tree::ptree &) const;
    };
    /* END VIDEO SUB DESCRIPTORS */
    
    // undefined zone ... don't go there

    /* START AUDIO SUB DESCRIPTORS */
    // base class for MCA Audio SubDescriptors
    struct MCALabelSubDescriptor : public TransferFileBase
    {
        std::string mcaLinkId;              // UUID 
        std::string mcaLabelDictionaryId;   // UL
        std::string mcaTagName;             // string
        std::string mcaTagSymbol;           // string
        std::string rfc5646SpokenLanguage;         // locale
    
        // OPTIONAL? TO-DO: Check this
        std::string mcaAudioElementKind;    // string
        std::string mcaAudioContentKind;    // string
        std::string mcaTitle;               // string
        std::string mcaTitleVersion;        // string

        MCALabelSubDescriptor() : TransferFileBase() {};
        virtual ~MCALabelSubDescriptor() {};

        virtual void Write(boost::property_tree::ptree &) const;
    };

    struct AudioChannelLabelSubDescriptor : public MCALabelSubDescriptor
    {
        std::string soundfieldGroupLinkId;  // UUID
        uint16_t mcaChannelId;

        AudioChannelLabelSubDescriptor() : MCALabelSubDescriptor() {};
        virtual ~AudioChannelLabelSubDescriptor() {};

        virtual void Write(boost::property_tree::ptree &) const;
    
        virtual std::string GetTypeAsString() const { return "AudioChannelLabelSubDescriptor"; };
    };

    struct SoundfieldGroupLabelSubDescriptor : public MCALabelSubDescriptor
    {

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
