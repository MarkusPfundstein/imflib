#include "mxfreader.h"

#include <Metadata.h>
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

bool readTransferFileBase(ASDCP::MXF::InterchangeObject *gen, std::shared_ptr<IMFEssenceDescriptor::TransferFileBase> in);

bool readMXFEssenceDescriptorBase(ASDCP::MXF::FileDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::MXFEssenceDescriptorBase> in);

bool readVideoEssenceDescriptor(ASDCP::MXF::GenericPictureEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::VideoEssenceDescriptor> in);

bool readRGBAEssenceDescriptor(ASDCP::MXF::RGBAEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::RGBAEssenceDescriptor> in);

bool readCDCIEssenceDescriptor(ASDCP::MXF::CDCIEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::CDCIEssenceDescriptor> in);

bool readAudioEssenceDescriptor(ASDCP::MXF::GenericSoundEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::AudioEssenceDescriptor> in);

bool readWaveAudioDescriptor(ASDCP::MXF::WaveAudioDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::WaveAudioDescriptor> in);

/* START AUDIO SUB DESCRIPTOR READ METHODS */
// TO-DO: SubDescriptor functions will also call readTransferFileBase and functions for subclasses. Should be fixed for functions above
bool readMCALabelSubDescriptor(ASDCP::MXF::MCALabelSubDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::MCALabelSubDescriptor> in);

bool readAudioChannelLabelSubDescriptor(ASDCP::MXF::AudioChannelLabelSubDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::AudioChannelLabelSubDescriptor> in);

bool readSoundfieldGroupLabelSubDescriptor(ASDCP::MXF::SoundfieldGroupLabelSubDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::SoundfieldGroupLabelSubDescriptor> in);

bool readJPEG2000SubDescriptor(ASDCP::MXF::JPEG2000PictureSubDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::JPEG2000SubDescriptor> in);

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

    ASDCP::WriterInfo info;
    result = reader.FillWriterInfo(info);
    if (KM_FAILURE(result)) {
        reader.Close();
        throw MXFReaderException("Error reading AudioFile");
    }

    waveDescriptor = dynamic_cast<ASDCP::MXF::WaveAudioDescriptor*>(temp);
    if (waveDescriptor == nullptr) {
        reader.Close();
        throw MXFReaderException("No WaveEssence descriptor found");
    }

    waveDescriptor->Dump();

    std::shared_ptr<IMFEssenceDescriptor::WaveAudioDescriptor> imfWaveAudioDescriptor(
        new IMFEssenceDescriptor::WaveAudioDescriptor());

    bool s = readTransferFileBase(waveDescriptor, imfWaveAudioDescriptor);
    s = s && readMXFEssenceDescriptorBase(waveDescriptor, imfWaveAudioDescriptor);
    s = s && readAudioEssenceDescriptor(waveDescriptor, imfWaveAudioDescriptor);
    s = s && readWaveAudioDescriptor(waveDescriptor, imfWaveAudioDescriptor);
    if (!s) {
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
    std::list<std::shared_ptr<IMFEssenceDescriptor::AudioChannelLabelSubDescriptor>> audioLabelSubDescriptors;
    std::list<std::shared_ptr<IMFEssenceDescriptor::SoundfieldGroupLabelSubDescriptor>> soundfieldGroupLabelSubDescriptors;

    // to collect we need to put all stuff into an object list. Then we can dynamic cast (urks);
    std::list<ASDCP::MXF::InterchangeObject*> tmpListSubDescriptors;

    reader.OP1aHeader().GetMDObjectsByType(
        ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_AudioChannelLabelSubDescriptor),
        tmpListSubDescriptors);
    reader.OP1aHeader().GetMDObjectsByType(
        ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_SoundfieldGroupLabelSubDescriptor), 
        tmpListSubDescriptors);
/*  NOT USED YET
    reader.OP1aHeader().GetMDObjectsByType(
        ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_GroupOfSoundfieldGroupsLabelSubDescriptor), 
        object_list);
*/

    bool success = true;
    for (auto it = tmpListSubDescriptors.begin(); success && it != tmpListSubDescriptors.end(); ++it) {
        (*it)->Dump();
        auto *p = dynamic_cast<ASDCP::MXF::AudioChannelLabelSubDescriptor*>(*it);
        if (p) {
            std::shared_ptr<IMFEssenceDescriptor::AudioChannelLabelSubDescriptor> tmp(
                new IMFEssenceDescriptor::AudioChannelLabelSubDescriptor);

            success = success && readAudioChannelLabelSubDescriptor(p, tmp);

            imfWaveAudioDescriptor->subDescriptors.push_back(tmp);
        } else {
            auto p2 = dynamic_cast<ASDCP::MXF::SoundfieldGroupLabelSubDescriptor*>(*it);
            if (p2) {
                std::shared_ptr<IMFEssenceDescriptor::SoundfieldGroupLabelSubDescriptor> tmp(
                    new IMFEssenceDescriptor::SoundfieldGroupLabelSubDescriptor);

                success = success && readSoundfieldGroupLabelSubDescriptor(p2, tmp);

                imfWaveAudioDescriptor->subDescriptors.push_back(tmp);
            } else {
                success = false;
            }
        }
    }

    if (!success 
        || tmpListSubDescriptors.empty() 
        || tmpListSubDescriptors.size() != imfWaveAudioDescriptor->subDescriptors.size()) {
        reader.Close();
        throw MXFReaderException("Error with audio MCALabelSubDescriptors and so.. check logs");
    }

    // TO-DO: Don't make random EssenceDescriptor UUID every time we open an IMP. Keep the old one if IMP exists
    std::shared_ptr<IMFEssenceDescriptor> essenceDescriptor(
        new IMFEssenceDescriptor(UUIDGenerator().MakeUUID(), imfWaveAudioDescriptor));

    char strBuf[IDENT_BUFFER_LENGTH];
    *strBuf = 0;
    std::string trackId = ASDCP::UUID(info.AssetUUID).EncodeHex(strBuf, IDENT_BUFFER_LENGTH);
    std::shared_ptr<IMFAudioTrack> track(new IMFAudioTrack(trackId, _filename, essenceDescriptor));
    track->SetBits(waveDescriptor->QuantizationBits);
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
        
        bool s = readTransferFileBase(rgbaDescriptor, videoEssenceDescriptor);
        s = s && readMXFEssenceDescriptorBase(rgbaDescriptor, videoEssenceDescriptor);
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
            bool s = readTransferFileBase(cdciDescriptor, videoEssenceDescriptor);
            s = s && readMXFEssenceDescriptorBase(cdciDescriptor, videoEssenceDescriptor);
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

    // Done with parsing RGBA/CDCI. Get PictureSub
    ASDCP::MXF::JPEG2000PictureSubDescriptor *pictureSubDescriptor = nullptr;
    result = reader.OP1aHeader().GetMDObjectByType(
		ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_JPEG2000PictureSubDescriptor),
		reinterpret_cast<ASDCP::MXF::InterchangeObject**>(&pictureSubDescriptor));
    if (!KM_SUCCESS(result)) {
        reader.Close();
        throw MXFReaderException("COULDNT FIND JPEG2000PictureSubDescriptor");
    }
    pictureSubDescriptor->Dump();

    // Read PictureSub
    std::shared_ptr<IMFEssenceDescriptor::JPEG2000SubDescriptor> jpeg2000SubDescriptor(new IMFEssenceDescriptor::JPEG2000SubDescriptor());
    if (!readJPEG2000SubDescriptor(pictureSubDescriptor, jpeg2000SubDescriptor)) {
        reader.Close();
        throw MXFReaderException("Error parsing JPEG2000PictureSubDescriptor");
    }

    videoEssenceDescriptor->subDescriptors.push_back(jpeg2000SubDescriptor);

    // Just a test if we have track files
    std::list<ASDCP::MXF::InterchangeObject*> objectList;
    reader.OP1aHeader().GetMDObjectsByType(ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_Track), objectList);
    if (objectList.empty()) {
        reader.Close();
        throw MXFReaderException("NO TRACK FILES IN MXF");
    }

    // TO-DO:
    // if we re-open a IMP, parse this from CPL and set accordingly. Now we generate one randomly every time
    // we open. works, but ugly. And probably ................... WILL LEAD TO A HARD TO TRACK DOWN BUG
    std::shared_ptr<IMFEssenceDescriptor> essenceDescriptor(new IMFEssenceDescriptor(UUIDGenerator().MakeUUID(), videoEssenceDescriptor));

    char strBuf[41];
    std::string trackId =  ASDCP::UUID(info.AssetUUID).EncodeHex(strBuf, 40);

    std::shared_ptr<IMFVideoTrack> track(new IMFVideoTrack(trackId, _filename, essenceDescriptor));

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

bool readTransferFileBase(ASDCP::MXF::InterchangeObject *gen, std::shared_ptr<IMFEssenceDescriptor::TransferFileBase> in)
{
    char identbuf[IDENT_BUFFER_LENGTH];
    *identbuf = 0;

    in->instanceId = gen->InstanceUID.EncodeString(identbuf, IDENT_BUFFER_LENGTH);

    return true;
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
    // TO-DO: Don't use string but chars
    in->pixelLayout = gen->PixelLayout.EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    return true;
}

bool readCDCIEssenceDescriptor(ASDCP::MXF::CDCIEssenceDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::CDCIEssenceDescriptor> in)
{
    // horizontal/componentdepth are only mandatary in asdcplib
    // TO-DO: make check for the other ones
    in->componentDepth = gen->ComponentDepth;
    in->horizontalSubsampling = gen->HorizontalSubsampling;
    in->verticalSubsampling = gen->VerticalSubsampling.get();
    in->whiteRefLevel = gen->WhiteReflevel.get();
    in->blackRefLevel = gen->BlackRefLevel.get();

    return true;
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

bool readMCALabelSubDescriptor(ASDCP::MXF::MCALabelSubDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::MCALabelSubDescriptor> in)
{
    bool success = readTransferFileBase(gen, in);

    char identbuf[IDENT_BUFFER_LENGTH];
    *identbuf = 0;             

    if (gen->MCATagName.empty()) {
        PrintIMFComplianceWarning("MCATagName");
        return false;
    }
    if (gen->RFC5646SpokenLanguage.empty()) {
        PrintIMFComplianceWarning("RFC5646SpokenLanguage");
        return false;
    }

    in->mcaLinkId = gen->MCALinkID.EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    in->mcaLabelDictionaryId = gen->MCALabelDictionaryID.EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    in->mcaTagSymbol = gen->MCATagSymbol.EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    in->mcaTagName = gen->MCATagName.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    in->rfc5646SpokenLanguage = gen->RFC5646SpokenLanguage.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);

    // TO-DO: Make them mandatory and add code to asdcplib
    if (!gen->MCAAudioElementKind.empty()) {
        in->mcaAudioElementKind = gen->MCAAudioElementKind.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    }
    if (!gen->MCAAudioContentKind.empty()) {
        in->mcaAudioContentKind = gen->MCAAudioContentKind.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    }
    if (!gen->MCATitle.empty()) {
        in->mcaTitle = gen->MCATitle.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    }
    if (!gen->MCATitleVersion.empty()) {
        in->mcaTitleVersion = gen->MCATitleVersion.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    }

    return success;
}

bool readAudioChannelLabelSubDescriptor(ASDCP::MXF::AudioChannelLabelSubDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::AudioChannelLabelSubDescriptor> in)
{
    bool success = readMCALabelSubDescriptor(gen, in);

    if (gen->MCAChannelID.empty()) {
        PrintIMFComplianceWarning("MCAChannelID");
        return false;
    }
    if (gen->SoundfieldGroupLinkID.empty()) {
        PrintIMFComplianceWarning("SoundfieldGroupLinkID");
        return false;
    }

    char identbuf[IDENT_BUFFER_LENGTH];
    *identbuf = 0;             

    in->mcaChannelId = gen->MCAChannelID.get();
    in->soundfieldGroupLinkId = gen->SoundfieldGroupLinkID.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);

    return success;
}

bool readSoundfieldGroupLabelSubDescriptor(ASDCP::MXF::SoundfieldGroupLabelSubDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::SoundfieldGroupLabelSubDescriptor> in)
{
    bool success = readMCALabelSubDescriptor(gen, in);

    return success;
}

bool readJPEG2000SubDescriptor(ASDCP::MXF::JPEG2000PictureSubDescriptor *gen, std::shared_ptr<IMFEssenceDescriptor::JPEG2000SubDescriptor> in)
{
    bool success = readTransferFileBase(gen, in);

    if (gen->PictureComponentSizing.empty()) {
        PrintIMFComplianceWarning("PictureComponentSizing");
        return false;
    }
    if (gen->CodingStyleDefault.empty()) {
        PrintIMFComplianceWarning("CodingStyleDefault");
        return false;
    }
    if (gen->QuantizationDefault.empty()) {
        PrintIMFComplianceWarning("QuantizationDefault");
        return false;
    }
    if (gen->J2CLayout.empty()) {
        PrintIMFComplianceWarning("J2CLayout");
        return false;
    }

    char identbuf[IDENT_BUFFER_LENGTH];
    *identbuf = 0;

    in->rsize = gen->Rsize;
    in->xsize = gen->Xsize;
    in->ysize = gen->Ysize;
    in->xOsize = gen->XOsize;
    in->yOsize = gen->YOsize;
    in->xTsize = gen->XTsize;
    in->yTsize = gen->YTsize;
    in->xTOsize = gen->XTOsize;
    in->yTOsize = gen->YTOsize;
    in->csize = gen->Csize;

    // TO-DO: Don't use string but chars
    in->pictureComponentSizing = gen->PictureComponentSizing.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    in->codingStyleDefault = gen->CodingStyleDefault.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    in->quantizationDefault = gen->QuantizationDefault.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);
    // TO-DO: Don't use string but chars
    in->j2cLayout = gen->J2CLayout.get().EncodeString(identbuf, IDENT_BUFFER_LENGTH);

    return success;
}

