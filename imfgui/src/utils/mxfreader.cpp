#include "mxfreader.h"

#include <AS_02.h>
#include <iostream>
#include <sstream>

#include "../model/imfvideotrack.h"
#include "../model/imfaudiotrack.h"

#include <KM_util.h>

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

void MXFReader::ParseMetadata(const std::shared_ptr<IMFAudioTrack> &track)
{
    AS_02::PCM::MXFReader reader;
    ASDCP::MXF::WaveAudioDescriptor *waveDescriptor = nullptr;

    ASDCP::Result_t result = reader.OpenRead(track->GetPath());
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


    int duration = 0;
    if (waveDescriptor->ContainerDuration.get() == 0 ) {
        std::cout << "[Warning] ContainerDuration not set in file descriptor, attempting to use index duration." << std::endl;
        duration = reader.AS02IndexReader().GetDuration();
    } else {
        duration = waveDescriptor->ContainerDuration;

    }
    if (duration == 0) {
        std::cout << "[Warning] Couldn't get duration" << std::endl;
        duration = -1;
    }

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

    track->SetUUID(ss.str());
    track->SetBits(bits);
    track->SetDuration(duration);
    track->SetEditRate(RationalNumber(waveDescriptor->SampleRate.Numerator, waveDescriptor->SampleRate.Denominator));
    reader.Close();
}

void MXFReader::ParseMetadata(const std::shared_ptr<IMFVideoTrack> &track)
{
    AS_02::JP2K::MXFReader reader;
    ASDCP::MXF::RGBAEssenceDescriptor *rgbaDescriptor = nullptr;
    ASDCP::MXF::CDCIEssenceDescriptor *cdciDescriptor = nullptr;

    ASDCP::Result_t result = reader.OpenRead(track->GetPath());
    if (KM_FAILURE(result)) {
        throw MXFReaderException("Error opening file");
    }

    result = reader.OP1aHeader().GetMDObjectByType(ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_RGBAEssenceDescriptor),
						     reinterpret_cast<ASDCP::MXF::InterchangeObject**>(&rgbaDescriptor));

    //reader.OP1aHeader().Dump();

    // get used down below to set color space etc
    int bits = -1;
    int duration = -1;
    ASDCP::Rational editRate;
    IMFVideoTrack::IMF_COLOR_SPACE colorSpace = IMFVideoTrack::IMF_COLOR_SPACE::INVALID;

    if (KM_SUCCESS(result)) {
        // we have a rgba descriptor
        colorSpace = IMFVideoTrack::IMF_COLOR_SPACE::RGB444;
        duration = rgbaDescriptor->ContainerDuration;
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

        //rgbaDescriptor->Dump();
        /*
        Kumu::UUID uuid = rgbaDescriptor->InstanceUID;
        char buf[41];
        uuid.EncodeHex(buf, 40);
        std::cout << "buf: " << buf << std::endl;
        */
    } else {
        // check if we have a cdci (yuv) descriptor
        result = reader.OP1aHeader().GetMDObjectByType(ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_CDCIEssenceDescriptor),
							 reinterpret_cast<ASDCP::MXF::InterchangeObject**>(&cdciDescriptor));

        if (KM_SUCCESS(result)) {
            if ((cdciDescriptor->HorizontalSubsampling == 1)) {
                colorSpace = IMFVideoTrack::IMF_COLOR_SPACE::YUV444;
            } else if (cdciDescriptor->HorizontalSubsampling == 2) {
                colorSpace = IMFVideoTrack::IMF_COLOR_SPACE::YUV422;
            }
            duration = cdciDescriptor->ContainerDuration;
            editRate = cdciDescriptor->SampleRate;
            bits = cdciDescriptor->ComponentDepth;

        } else {
            reader.Close();
            throw MXFReaderException("No essence descriptor found");
        }
        /*Kumu::UUID uuid = cdciDescriptor->InstanceUID;
        char buf[41];
        uuid.EncodeHex(buf, 40);
        std::cout << "buf: " << buf << std::endl;*/
    }

    /* Lets try to get SourceEncoding */
    /*
    ASDCP::MXF::SourcePackage *sourcePackage = reader.OP1aHeader().GetSourcePackage();
    Kumu::UUID uuid = sourcePackage->Descriptor;
    char buf[41];
    uuid.EncodeHex(buf, 40);
    std::cout << "buf: " << buf << std::endl;
    */

    ASDCP::WriterInfo info;
    result = reader.FillWriterInfo(info);
    if (KM_FAILURE(result)) {
        reader.Close();
        throw MXFReaderException("Couldn't get UUID");
    }

    char strBuf[41];
    std::stringstream ss;
    ss << ASDCP::UUID(info.AssetUUID).EncodeHex(strBuf, 40);

    track->SetUUID(ss.str());
    track->SetBits(bits);
    track->SetColorSpace(colorSpace);
    track->SetDuration(duration);
    track->SetEditRate(RationalNumber(editRate.Numerator, editRate.Denominator));

    reader.Close();
}

//void MXFReader::ReadHeader()
