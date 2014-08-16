#include "mxfreader.h"

#include <AS_02.h>
#include <iostream>
#include <sstream>

#include "../model/imfvideotrack.h"
#include "../model/imfaudiotrack.h"

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

        if ((int)rgbaDescriptor->ComponentMinRef == 0) {
            // full range only with comp. min ref = 0
            bits = std::log((int)rgbaDescriptor->ComponentMaxRef + 1) / std::log(2);
        } else {
            // TO-DO:
            bits = -1;
        }

        //rgbaDescriptor->Dump();
        //reader.OP1aHeader().Dump();
        /*
        ASDCP::MXF::FileDescriptor *file;


        result = reader.OP1aHeader().GetMDObjectByType(ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_FileDescriptor),
							 reinterpret_cast<ASDCP::MXF::InterchangeObject**>(&file));
        if (KM_SUCCESS(result)) {
            file->Dump();
        }
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

    track->SetUUID(ss.str());
    track->SetBits(bits);
    track->SetColorSpace(colorSpace);
    track->SetDuration(duration);
    track->SetEditRate(RationalNumber(editRate.Numerator, editRate.Denominator));

    reader.Close();
}

//void MXFReader::ReadHeader()
