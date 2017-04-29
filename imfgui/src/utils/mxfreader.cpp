#include "mxfreader.h"

#include <AS_02.h>
#include <iostream>
#include <sstream>
#include <list>

#include "../model/imfvideotrack.h"
#include "../model/imfaudiotrack.h"
#include "uuidgenerator.h"

#include <KM_util.h>

// TO-DO: Point to value defined in asdcplib KLV.h 
const uint32_t IDENT_BUFFER_LENGTH = 128;

bool readMXFEssenceDescriptorBase(ASDCP::MXF::FileDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::MXFEssenceDescriptorBase> in);

bool readVideoEssenceDescriptor(ASDCP::MXF::GenericPictureEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::VideoEssenceDescriptor> in);

bool readRGBAEssenceDescriptor(ASDCP::MXF::RGBAEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::RGBAEssenceDescriptor> in);

bool readCDCIEssenceDescriptor(ASDCP::MXF::CDCIEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::CDCIEssenceDescriptor> in);

bool readAudioEssenceDescriptor(ASDCP::MXF::GenericSoundEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::AudioEssenceDescriptor> in);

bool readWaveAudioDescriptor(ASDCP::MXF::WaveAudioDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::WaveAudioDescriptor> in);


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

    if (!readAudioEssenceDescriptor(waveDescriptor, imfWaveAudioDescriptor)) {
        reader.Close();
        throw MXFReaderException("something wrong with GenericSoundDescriptor... check logs");
    }
    if (!readWaveAudioDescriptor(waveDescriptor, imfWaveAudioDescriptor)) {
        throw MXFReaderException("something wrong with WaveAudioDescriptor... check logs");
    }   

    int duration = 0;
    // On some clipster beta files this was indeed not set.
    if (waveDescriptor->ContainerDuration.empty()) {
        std::cout << "[Warning] ContainerDuration not set in file descriptor, attempting to use index duration." << std::endl; duration = reader.AS02IndexReader().GetDuration(); } else {
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

    std::shared_ptr<IMFEssenceDescriptor::VideoEssenceDescriptor> videoEssenceDescriptor;

    ASDCP::Result_t result = reader.OpenRead(_filename);
    if (KM_FAILURE(result)) {
        throw MXFReaderException("Error opening file");
    }

    ASDCP::WriterInfo info;
    result = reader.FillWriterInfo(info);
    if (KM_FAILURE(result)) {
        reader.Close();
        throw MXFReaderException("Couldn't read file");
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

        videoEssenceDescriptor.reset(
                new IMFEssenceDescriptor::RGBAEssenceDescriptor());

        // TO-DO move this up. can work for CDCI as well
        
        bool s = readMXFEssenceDescriptorBase(rgbaDescriptor, videoEssenceDescriptor);
        s = s && readVideoEssenceDescriptor(rgbaDescriptor, videoEssenceDescriptor);
        s = s && readRGBAEssenceDescriptor(rgbaDescriptor, std::dynamic_pointer_cast<IMFEssenceDescriptor::RGBAEssenceDescriptor>(videoEssenceDescriptor));
        if (!s) {
            reader.Close();
            throw MXFReaderException("somewthing wrong with PictureEssence (RGBA) ... check logs");
        }

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
        // check if we have a cdci (yuv) descriptor
        result = reader.OP1aHeader().GetMDObjectByType(
		    ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_CDCIEssenceDescriptor),
		    reinterpret_cast<ASDCP::MXF::InterchangeObject**>(&cdciDescriptor));

        if (KM_SUCCESS(result)) {
            // TO-DO: MOVE THAT UP
            videoEssenceDescriptor.reset(
                    new IMFEssenceDescriptor::CDCIEssenceDescriptor());

            // TO-DO move this up. can work for CDCI as well
            bool s = readMXFEssenceDescriptorBase(cdciDescriptor, videoEssenceDescriptor);
            s = s && readVideoEssenceDescriptor(cdciDescriptor, videoEssenceDescriptor);
            s = s && readCDCIEssenceDescriptor(cdciDescriptor, std::dynamic_pointer_cast<IMFEssenceDescriptor::CDCIEssenceDescriptor>(videoEssenceDescriptor));
            if (!s) {
                reader.Close();
                throw MXFReaderException("something wrong with PicturEssence (CDCI) ... check logs");
            }
                
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
        }
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
    std::shared_ptr<IMFEssenceDescriptor> essenceDescriptor(new IMFEssenceDescriptor(uuid, videoEssenceDescriptor));

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

void PrintIMFComplianceWarning(std::string s)
{
    std::cerr << "[IMF WARNING] Field: " << s << " missing from MXF. File might not pass Photon" << std::endl;
}

bool readMXFEssenceDescriptorBase(ASDCP::MXF::FileDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::MXFEssenceDescriptorBase> in)
{
    char identbuf[IDENT_BUFFER_LENGTH];
    *identbuf = 0;

    if (gen->ContainerDuration.empty()) {
        PrintIMFComplianceWarning("ContainerDuration");
        return false;
    }
    if (gen->LinkedTrackID.empty()) {
        PrintIMFComplianceWarning("LinkedTrackID");
        return false;
    }

    in->instanceId = gen->InstanceUID.EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    in->sampleRate = RationalNumber(gen->SampleRate.Numerator, gen->SampleRate.Denominator);
    in->containerFormat = gen->EssenceContainer.EncodeString(identbuf, IDENT_BUFFER_LENGTH);

    in->essenceLength = gen->ContainerDuration.get();
    in->linkedTrackId = gen->LinkedTrackID.get();
    return true;
}

bool readVideoEssenceDescriptor(ASDCP::MXF::GenericPictureEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::VideoEssenceDescriptor> in)
{
    char identbuf[IDENT_BUFFER_LENGTH];
    *identbuf = 0;
    if (gen->TransferCharacteristic.empty()) {
        PrintIMFComplianceWarning("TransferCharacteristic");
        return false;
    } 
    if (gen->CodingEquations.empty()) {
        PrintIMFComplianceWarning("CodingEquations");
        return false;
    }
    if (gen->ColorPrimaries.empty()) {
        PrintIMFComplianceWarning("ColorPrimaries");
        return false;
    }

    in->colorPrimaries = gen->ColorPrimaries.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    in->codingEquations = gen->CodingEquations.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    in->transferCharacteristic = gen->TransferCharacteristic.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    in->pictureCompression = gen->PictureEssenceCoding.EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    in->imageAspectRatio = RationalNumber(gen->AspectRatio.Numerator, gen->AspectRatio.Denominator);
    in->frameLayout = gen->FrameLayout;    
    in->storedWidth = gen->StoredWidth;
    in->storedHeight = gen->StoredHeight;
    return true;
}

bool readRGBAEssenceDescriptor(ASDCP::MXF::RGBAEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::RGBAEssenceDescriptor> in)
{
    char identbuf[IDENT_BUFFER_LENGTH];
    *identbuf = 0;             

    if (gen->ComponentMaxRef.empty()) {      
        PrintIMFComplianceWarning("ComponentMaxRef");
        return false;
    }
    if (gen->ComponentMinRef.empty()) {  
        PrintIMFComplianceWarning("ComponentMinRef");
        return false;
    }

    in->componentMaxRef = gen->ComponentMaxRef.get(); 
    in->componentMinRef = gen->ComponentMinRef.get();
    return true;
}

bool readCDCIEssenceDescriptor(ASDCP::MXF::CDCIEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::CDCIEssenceDescriptor> in)
{
    std::cout << "[WARNING] CDCIEssenceDescriptor reading N.S.Y" << std::endl;
    return false;
}

bool readAudioEssenceDescriptor(ASDCP::MXF::GenericSoundEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::AudioEssenceDescriptor> in)
{
    char identbuf[IDENT_BUFFER_LENGTH];
    *identbuf = 0;             

    in->soundCompression = gen->SoundEssenceCoding.EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    in->audioSampleRate = RationalNumber(gen->AudioSamplingRate.Numerator, gen->AudioSamplingRate.Denominator);
    in->channelCount = gen->ChannelCount;
    in->quantizationBits = gen->QuantizationBits;
    in->locked = gen->Locked;

    return true;
}

bool readWaveAudioDescriptor(ASDCP::MXF::WaveAudioDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::WaveAudioDescriptor> in)
{
    char identbuf[IDENT_BUFFER_LENGTH];
    *identbuf = 0;             
    if (gen->ChannelAssignment.empty()) {
        PrintIMFComplianceWarning("ChannelAssignment");
        return false;
    }

    in->blockAlign = gen->BlockAlign;
    in->averageBytesPerSecond = gen->AvgBps;
    in->channelAssignment = gen->ChannelAssignment.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    return true;
}


