#include "mxfreader.h"

#include <AS_02.h>
#include <iostream>

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

    ASDCP::Result_t result = reader.OpenRead(track->GetFileName());
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

    track->SetDuration(duration);
    track->SetEditRate(RationalNumber(waveDescriptor->SampleRate.Numerator, waveDescriptor->SampleRate.Denominator));
    reader.Close();
}

void MXFReader::ParseMetadata(const std::shared_ptr<IMFVideoTrack> &track)
{
    AS_02::JP2K::MXFReader reader;
    ASDCP::MXF::RGBAEssenceDescriptor *rgbaDescriptor = nullptr;
    ASDCP::MXF::CDCIEssenceDescriptor *cdciDescriptor = nullptr;

    ASDCP::Result_t result = reader.OpenRead(track->GetFileName());
    if (KM_FAILURE(result)) {
        throw MXFReaderException("Error opening file");
    }

    result = reader.OP1aHeader().GetMDObjectByType(ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_RGBAEssenceDescriptor),
						     reinterpret_cast<ASDCP::MXF::InterchangeObject**>(&rgbaDescriptor));

    int duration = 0;
    ASDCP::Rational editRate;
    if (KM_SUCCESS(result)) {
        // we have a rgba descriptor
        duration = rgbaDescriptor->ContainerDuration;
        editRate = rgbaDescriptor->SampleRate;
    } else {
        // check if we have a cdci (yuv) descriptor
        result = reader.OP1aHeader().GetMDObjectByType(ASDCP::DefaultCompositeDict().ul(ASDCP::MDD_CDCIEssenceDescriptor),
							 reinterpret_cast<ASDCP::MXF::InterchangeObject**>(&cdciDescriptor));

        if (KM_SUCCESS(result)) {
            duration = cdciDescriptor->ContainerDuration;
            editRate = cdciDescriptor->SampleRate;
        } else {
            reader.Close();
            throw MXFReaderException("No essence descriptor found");
        }
    }

    track->SetDuration(duration);
    track->SetEditRate(RationalNumber(editRate.Numerator, editRate.Denominator));

    reader.Close();
}

//void MXFReader::ReadHeader()
