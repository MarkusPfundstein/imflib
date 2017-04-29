#include "mxfreader.h"

#include <AS_02.h>
#include <iostream>
#include <sstream>
#include <list>

#include "../model/imfvideotrack.h"
#include "../model/imfaudiotrack.h"
#include "uuidgenerator.h"

#include <KM_util.h>

MXFReader::MXFReader(const std::string& filename)
    : 
    _filename(filename)
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

std::shared_ptr<IMFAudioTrack> MXFReader::ReadAudioTrack()
{
    AS_02::PCM::MXFReader reader;
    ASDCP::MXF::WaveAudioDescriptor *waveDescriptor = nullptr;

    ASDCP::Result_t result = reader.OpenRead(_filename);
    if (KM_FAILURE(result)) {
        throw MXFReaderException("Error opening file");
    }

    ASDCP::MXF::InterchangeObject* temp = nullptr;

    result = reader.OP1aHeader().GetMDObjectByType(ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_WaveAudioDescriptor), &temp);
    if (KM_FAILURE(result)) {
        reader.Close();
        throw MXFReaderException("Error reading OP1aHeader");
    }

    waveDescriptor = dynamic_cast<ASDCP::MXF::WaveAudioDescriptor*>(temp);
    if (waveDescriptor == nullptr) {
        reader.Close();
        throw MXFReaderException("No essence descriptor found");
    }

    waveDescriptor->Dump();

    std::shared_ptr<IMFEssenceDescriptor::WaveAudioDescriptor> imfWaveAudioDescriptor(
        new IMFEssenceDescriptor::WaveAudioDescriptor());

    int duration = 0;
    // On some clipster beta files this was indeed not set.
    if (waveDescriptor->ContainerDuration.empty()) {
        std::cout << "[Warning] ContainerDuration not set in file descriptor, attempting to use index duration." << std::endl;
        duration = reader.AS02IndexReader().GetDuration();
    } else {
        duration = waveDescriptor->ContainerDuration;
    }

    //int duration = reader.AS02IndexReader().GetDuration();
    if (duration == 0) {
        std::cout << "[Warning] Couldn't get duration" << std::endl;
        duration = -1;
    }


    // read all the subdescriptors
    std::list<ASDCP::MXF::InterchangeObject*> object_list;
    reader.OP1aHeader().GetMDObjectsByType(
        ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_AudioChannelLabelSubDescriptor), 
        object_list);
    reader.OP1aHeader().GetMDObjectsByType(
        ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_SoundfieldGroupLabelSubDescriptor), 
        object_list);
    reader.OP1aHeader().GetMDObjectsByType(
        ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_GroupOfSoundfieldGroupsLabelSubDescriptor), 
        object_list);

    std::list<ASDCP::MXF::InterchangeObject*>::iterator i = object_list.begin();
    for ( ; i != object_list.end(); ++i ) 
    {
        ASDCP::MXF::MCALabelSubDescriptor *p = dynamic_cast<ASDCP::MXF::MCALabelSubDescriptor*>(*i);

        if (p != nullptr) {
            std::cout << "found subdescriptor" << std::endl;
            p->Dump();
        } else {
            char buf[64];  
            std::cerr << "Audio sub-descriptor type error.\n" << (**i).InstanceUID.EncodeHex(buf, 64) << std::endl;
        }
    }

    
    /*
    char identBuf[128];
    *identBuf = 0;
    essenceDescriptorData.uuid = waveDescriptor->EssenceContainer.EncodeString(identBuf, 128);
    */

    // no idea why this is here. but it checks for an error, so I leave it
    ASDCP::WriterInfo info;
    result = reader.FillWriterInfo(info);
    if (KM_FAILURE(result)) {
        reader.Close();
        throw MXFReaderException("Couldn't get UUID");
    }

    int bits = waveDescriptor->QuantizationBits;

    char strBuf[41];
    std::stringstream ss;
    ss << ASDCP::UUID(info.AssetUUID).EncodeHex(strBuf, 40);

    std::string uuid = UUIDGenerator().MakeUUID();
    std::shared_ptr<IMFEssenceDescriptor> essenceDescriptor(new IMFEssenceDescriptor(uuid, imfWaveAudioDescriptor));

    std::shared_ptr<IMFAudioTrack> track(new IMFAudioTrack(ss.str(), _filename, essenceDescriptor));
    track->SetBits(bits);
    track->SetDuration(duration);
    track->SetEditRate(RationalNumber(waveDescriptor->SampleRate.Numerator, waveDescriptor->SampleRate.Denominator));
    reader.Close();

    return track;
}

std::shared_ptr<IMFVideoTrack> MXFReader::ReadVideoTrack()
{
    AS_02::JP2K::MXFReader reader;
    ASDCP::MXF::RGBAEssenceDescriptor *rgbaDescriptor = nullptr;
    ASDCP::MXF::CDCIEssenceDescriptor *cdciDescriptor = nullptr;

    std::shared_ptr<IMFEssenceDescriptor::MXFEssenceDescriptorBase> baseDescriptor;

    ASDCP::Result_t result = reader.OpenRead(_filename);
    if (KM_FAILURE(result)) {
        throw MXFReaderException("Error opening file");
    }

    // get used down below to set color space etc
    int bits = -1;
    int duration = -1;
    ASDCP::Rational editRate;
    IMFVideoTrack::IMF_COLOR_SPACE colorSpace = IMFVideoTrack::IMF_COLOR_SPACE::INVALID;

    result = reader.OP1aHeader().GetMDObjectByType(
        ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_RGBAEssenceDescriptor),
		reinterpret_cast<ASDCP::MXF::InterchangeObject**>(&rgbaDescriptor));
    if (KM_SUCCESS(result)) {
        // we have a rgba descriptor
        baseDescriptor.reset(
            static_cast<IMFEssenceDescriptor::MXFEssenceDescriptorBase*>(
                new IMFEssenceDescriptor::RGBAEssenceDescriptor()));

        colorSpace = IMFVideoTrack::IMF_COLOR_SPACE::RGB444;
        if (rgbaDescriptor->ContainerDuration.empty() == false) {
            duration = rgbaDescriptor->ContainerDuration;
        }
        editRate = rgbaDescriptor->SampleRate;

        int minRef = (int)rgbaDescriptor->ComponentMinRef;
        int maxRef = (int)rgbaDescriptor->ComponentMaxRef;

        // full range only with comp. min ref between 0 and 15 (SMPTE 274M-2008)
        if (minRef >= 0 && minRef <= 15) {
            bits = std::log(maxRef + 1) / std::log(2);
        } else if (minRef == 16 && maxRef == 235) {
            bits = 8;
        } else if (minRef == 64 && maxRef == 940) {
            bits = 10;
        } else if (minRef == 256 && maxRef == 3760) {
            bits = 12;
        } else {
            bits = -1;
        }

        rgbaDescriptor->Dump();
    } else {
        throw new MXFReaderException("YUV NOT SUPPORTED ATM");

/*
        // check if we have a cdci (yuv) descriptor
        result = reader.OP1aHeader().GetMDObjectByType(
		    ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_CDCIEssenceDescriptor),
		    reinterpret_cast<ASDCP::MXF::InterchangeObject**>(&cdciDescriptor));

        if (KM_SUCCESS(result)) {
	        essenceDescriptorData.type = IMFEssenceDescriptor::TYPE::CDCIEssenceDescriptor;

            if ((cdciDescriptor->HorizontalSubsampling == 1)) {
                colorSpace = IMFVideoTrack::IMF_COLOR_SPACE::YUV444;
            } else if (cdciDescriptor->HorizontalSubsampling == 2) {
                colorSpace = IMFVideoTrack::IMF_COLOR_SPACE::YUV422;
            }
            if (cdciDescriptor->ContainerDuration.empty() == false) {
                duration = cdciDescriptor->ContainerDuration;
            }
            editRate = cdciDescriptor->SampleRate;
            bits = cdciDescriptor->ComponentDepth;
            cdciDescriptor->Dump();

            //char identBuf[128];
            // *identBuf = 0;
            //cdciDescriptor->EssenceContainer.EncodeString(identBuf, 128);
        } else {
            reader.Close();
            throw MXFReaderException("No essence descriptor found");
        }*/
    }
    if (duration == -1) {
        std::cout << "[Warning] ContainerDuration not set in file descriptor, attempting to use index duration." << std::endl;
        duration = reader.AS02IndexReader().GetDuration();
        if (duration == 0) {
            duration = -1;
        }
    }

    // TO-DO:
    // if we re-open a IMP, parse this from CPL and set accordingly. Now we generate one randomly every time
    // we open. works, but ugly. And probably ................... WILL LEAD TO A HARD TO TRACK DOWN BUG
    std::string uuid = UUIDGenerator().MakeUUID();
    std::shared_ptr<IMFEssenceDescriptor> essenceDescriptor(new IMFEssenceDescriptor(uuid, baseDescriptor));

    ASDCP::MXF::JPEG2000PictureSubDescriptor *pictureSubDescriptor = nullptr;
    result = reader.OP1aHeader().GetMDObjectByType(
		ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_JPEG2000PictureSubDescriptor),
		reinterpret_cast<ASDCP::MXF::InterchangeObject**>(&pictureSubDescriptor));
    if (!KM_SUCCESS(result)) {
        reader.Close();
        throw MXFReaderException("COULDNT FIND JPEG2000PictureSubDescriptor");
    }
    pictureSubDescriptor->Dump();

    std::list<ASDCP::MXF::InterchangeObject*> objectList;
    reader.OP1aHeader().GetMDObjectsByType(ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_Track), objectList);
    if (objectList.empty()) {
        reader.Close();
        throw MXFReaderException("NO TRACK FILES IN MXF");
    }

    ASDCP::WriterInfo info;
    result = reader.FillWriterInfo(info);
    if (KM_FAILURE(result)) {
        reader.Close();
        throw MXFReaderException("Couldn't get UUID");
    }

    char strBuf[41];
    std::stringstream ss;
    ss << ASDCP::UUID(info.AssetUUID).EncodeHex(strBuf, 40);

    std::shared_ptr<IMFVideoTrack> track(new IMFVideoTrack(ss.str(), _filename, essenceDescriptor));
    track->SetBits(bits);
    track->SetColorSpace(colorSpace);
    track->SetDuration(duration);
    track->SetEditRate(RationalNumber(editRate.Numerator, editRate.Denominator));

    reader.Close();

    return track;
}

//void MXFReader::ReadHeader()
