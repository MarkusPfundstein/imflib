#include "mxfwriter.h"

#include <KM_fileio.h>
#include <KM_prng.h>
#include <AS_02.h>
#include <Metadata.h>
#include <iostream>

#include <stdexcept>
#include <memory>

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

void MXFWriter::MuxVideoFiles(const std::list<std::string> &files, const std::string& finalFile )
{
    AESEncContext* context = nullptr;
    HMACContext* HMAC = nullptr;
    AS_02::JP2K::MXFWriter writer;
    JP2K::FrameBuffer frameBuffer(4 * Kumu::Megabyte);
    JP2K::SequenceParser parser;
    //byte_t IV_buf[CBC_BLOCK_SIZE];
    Kumu::FortunaRNG rng;
    MXF::FileDescriptor *essenceDescriptor = nullptr;
    MXF::InterchangeObject_list_t essenceSubDescriptors;
    const Dictionary *dict = &DefaultSMPTEDict();

    Result_t result = parser.OpenRead(files, true);
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

    MXF::JPEG2000PictureSubDescriptor* pictureSubDescriptor = new MXF::JPEG2000PictureSubDescriptor(dict);

    bool yuvEssence = boost::any_cast<bool>(_muxerOptions["yuv_essence"]);
    if (yuvEssence) {
        std::cout << "write yuv essence" << std::endl;
        MXF::CDCIEssenceDescriptor* cdciDescriptor = new MXF::CDCIEssenceDescriptor(dict);
        essenceSubDescriptors.push_back(pictureSubDescriptor);

        result = JP2K_PDesc_to_MD(pictureDescriptor,
                                  *dict,
                                  *static_cast<MXF::GenericPictureEssenceDescriptor*>(cdciDescriptor),
                                  *static_cast<MXF::JPEG2000PictureSubDescriptor*>(essenceSubDescriptors.back()));

        if (ASDCP_SUCCESS(result)) {
            cdciDescriptor->PictureEssenceCoding = UL(dict->ul(MDD_JP2KEssenceCompression_BroadcastProfile_1));
            cdciDescriptor->HorizontalSubsampling = boost::any_cast<int>(_muxerOptions["subsampling_dx"]);
            cdciDescriptor->VerticalSubsampling = boost::any_cast<int>(_muxerOptions["subsampling_dy"]);
            cdciDescriptor->ComponentDepth = boost::any_cast<int>(_muxerOptions["bits"]);
            cdciDescriptor->FrameLayout = 0;    // no interlaced shit
            cdciDescriptor->AspectRatio = pictureDescriptor.AspectRatio;
            cdciDescriptor->FieldDominance = 0; // only for interlaced shit // field dominance = 1 -> upper field first
            essenceDescriptor = static_cast<MXF::FileDescriptor*>(cdciDescriptor);
	}
    } else {
        std::cout << "write rgb essence" << std::endl;
        MXF::RGBAEssenceDescriptor* rgbDescriptor = new MXF::RGBAEssenceDescriptor(dict);
        essenceSubDescriptors.push_back(pictureSubDescriptor);

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

    WriterInfo info = s_MyInfo;
    info.LabelSetType = LS_MXF_SMPTE;

    std::map<std::string, boost::any>::iterator it = _muxerOptions.find("asset_id");
    if (it != _muxerOptions.end()) {
        std::string assetId = boost::any_cast<std::string>(it->second);
        std::cout << "Use asset id " << assetId << std::endl;
        memcpy(info.AssetUUID, assetId.c_str(), UUIDlen);
    } else {
        std::cout << "Random UUID" << std::endl;
        Kumu::GenRandomUUID(info.AssetUUID);
    }

    result = writer.OpenWrite(finalFile,
                              info,
                              essenceDescriptor,
                              essenceSubDescriptors,
                              pictureDescriptor.EditRate,
                              16384, // mxf header size
                              AS_02::IS_FOLLOW, // index strategy
                              60); // partition space

    if (ASDCP_FAILURE(result)) {
        throw std::runtime_error("error open mxf writer");
    }

    result = parser.Reset();
    while (ASDCP_SUCCESS(result)) {
        result = parser.ReadFrame(frameBuffer);

        if (ASDCP_SUCCESS(result)) {
            //if ( Options.verbose_flag )
            //FrameBuffer.Dump(stderr, Options.fb_dump_size);
            //if ( Options.encrypt_header_flag )
            //FrameBuffer.PlaintextOffset(0);
            //}
        }

        if (ASDCP_SUCCESS(result)) {
            result = writer.WriteFrame(frameBuffer, context, HMAC);

            // The Writer class will forward the last block of ciphertext
            // to the encryption context for use as the IV for the next
            // frame. If you want to use non-sequitur IV values, un-comment
            // the following  line of code.
            // if ( ASDCP_SUCCESS(result) && Options.key_flag )
            //     Context->SetIVec(RNG.FillRandom(IV_buf, CBC_BLOCK_SIZE));
        }
    }

    if (result == RESULT_ENDOFFILE) {
        result = writer.Finalize();
        if (ASDCP_FAILURE(result)) {
            throw std::runtime_error("error writing mxf file");
        }
    }
}
