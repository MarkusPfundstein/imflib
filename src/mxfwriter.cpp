#include "mxfwriter.h"

#include <KM_fileio.h>
#include <KM_prng.h>
#include <AS_02.h>
#include <Metadata.h>
#include <iostream>

#include <stdexcept>

#include "rationalnumber.h"

using namespace ASDCP;

namespace ASDCP {
  Result_t JP2K_PDesc_to_MD(const ASDCP::JP2K::PictureDescriptor& PDesc,
			    const ASDCP::Dictionary& dict,
			    ASDCP::MXF::GenericPictureEssenceDescriptor& GenericPictureEssenceDescriptor,
			    ASDCP::MXF::JPEG2000PictureSubDescriptor& EssenceSubDescriptor);

  Result_t PCM_ADesc_to_MD(ASDCP::PCM::AudioDescriptor& ADesc, ASDCP::MXF::WaveAudioDescriptor* ADescObj);
}

class MyInfo : public WriterInfo {
public:
  MyInfo()
  {
      static byte_t default_ProductUUID_Data[UUIDlen] =
      { 0x7d, 0x83, 0x6e, 0x16, 0x37, 0xc7, 0x4c, 0x22,
        0xb2, 0xe0, 0x46, 0xa7, 0x17, 0xe8, 0x4f, 0x42 };

      memcpy(ProductUUID, default_ProductUUID_Data, UUIDlen);
      CompanyName = "ODMedia";
      ProductName = "IMF-Encoder PRE";
      ProductVersion = "0.1";
  }
} s_MyInfo;

MXFWriter::MXFWriter(const std::map<std::string, boost::any> &muxerOptions) :
    _muxerOptions(muxerOptions)
{
    //ctor
}

MXFWriter::~MXFWriter()
{
    //dtor
}

void MXFWriter::MuxVideoFiles(const std::string& inputDirectory, const std::string& finaleFile )
{
    AESEncContext* context = nullptr;
    HMACContext* HMAC = nullptr;
    AS_02::JP2K::MXFWriter writer;
    JP2K::FrameBuffer frameBuffer(4 * Kumu::Megabyte);
    JP2K::SequenceParser parser;
    byte_t IV_buf[CBC_BLOCK_SIZE];
    Kumu::FortunaRNG rng;
    MXF::FileDescriptor *essenceDescriptor = nullptr;
    MXF::InterchangeObject_list_t essenceSubDescriptors;
    const Dictionary *dict = &DefaultSMPTEDict();

    Result_t result = parser.OpenRead(inputDirectory.c_str(), true);
    if (ASDCP_SUCCESS(result) == 0) {
        throw std::runtime_error("error opening j2k parser");
    }

    RationalNumber aspectRatio = boost::any_cast<RationalNumber>(_muxerOptions["aspect_ratio"]);
    RationalNumber frameRate = boost::any_cast<RationalNumber>(_muxerOptions["framerate"]);

    JP2K::PictureDescriptor pictureDescriptor;
    parser.FillPictureDescriptor(pictureDescriptor);
    pictureDescriptor.EditRate = Rational(frameRate.num, frameRate.denum);
    pictureDescriptor.SampleRate = Rational(frameRate.num, frameRate.denum);
    pictureDescriptor.ContainerDuration = boost::any_cast<uint32_t>(_muxerOptions["container_duration"]);
    pictureDescriptor.AspectRatio = Rational(aspectRatio.num, aspectRatio.denum);

    JP2K::PictureDescriptorDump(pictureDescriptor);

    bool yuvEssence = boost::any_cast<bool>(_muxerOptions["yuv_essence"]);
    if (yuvEssence) {
        throw std::runtime_error("yuv muxing not possible yet");
        /*
        ASDCP::MXF::CDCIEssenceDescriptor* cdciDescriptor = new ASDCP::MXF::CDCIEssenceDescriptor(g_dict);
        essence_sub_descriptors.push_back(new ASDCP::MXF::JPEG2000PictureSubDescriptor(g_dict));

        result = ASDCP::JP2K_PDesc_to_MD(PDesc, *g_dict, *static_cast<ASDCP::MXF::GenericPictureEssenceDescriptor*>(cdciDescriptor),
					   *static_cast<ASDCP::MXF::JPEG2000PictureSubDescriptor*>(essence_sub_descriptors.back()));

        if ( ASDCP_SUCCESS(result) ) {
            cdciDescriptor->PictureEssenceCoding = Options.picture_coding;
            cdciDescriptor->HorizontalSubsampling = 1;
            cdciDescriptor->VerticalSubsampling = 1;
            cdciDescriptor->ComponentDepth = Options.component_depth;
            cdciDescriptor->FrameLayout = Options.frame_layout;
            cdciDescriptor->AspectRatio = Options.aspect_ratio;
            cdciDescriptor->FieldDominance = Options.field_dominance;
            essence_descriptor = static_cast<ASDCP::MXF::FileDescriptor*>(tmp_dscr);
	    }*/
    } else {
        MXF::RGBAEssenceDescriptor* rgbDescriptor = new MXF::RGBAEssenceDescriptor(dict);
        essenceSubDescriptors.push_back(new MXF::JPEG2000PictureSubDescriptor(dict));

        result = JP2K_PDesc_to_MD(pictureDescriptor,
                                  *dict,
                                  *static_cast<MXF::GenericPictureEssenceDescriptor*>(rgbDescriptor),
                                  *static_cast<MXF::JPEG2000PictureSubDescriptor*>(essenceSubDescriptors.back()));

        if (ASDCP_SUCCESS(result)) {
            rgbDescriptor->PictureEssenceCoding = UL(dict->ul(MDD_JP2KEssenceCompression_BroadcastProfile_1));
            rgbDescriptor->ComponentMaxRef = 1023;
            rgbDescriptor->ComponentMinRef = 0;
            essenceDescriptor = static_cast<MXF::FileDescriptor*>(rgbDescriptor);
	    }
    }

    if (essenceDescriptor == nullptr) {
        throw std::runtime_error("unable to obtain essence descriptor");
    }
}
