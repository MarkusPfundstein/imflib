#include "mxfwriter.h"

#include <KM_fileio.h>
#include <KM_prng.h>
#include <AS_02.h>
#include <Metadata.h>
#include <PCMParserList.h>
#include <iostream>

#include <stdexcept>
#include <memory>

#include "common.h"

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
      ProductName = "IMF-Encoder";
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

void MXFWriter::MuxAudioFile(const std::string& file, const std::string &finalFile)
{
    AESEncContext* Context = 0;
    HMACContext* HMAC = 0;
    PCMParserList Parser;
    AS_02::PCM::MXFWriter Writer;
    PCM::FrameBuffer FrameBuffer;
    byte_t IV_buf[CBC_BLOCK_SIZE];
    Kumu::FortunaRNG RNG;
    ASDCP::MXF::WaveAudioDescriptor *essence_descriptor = 0;
    const Dictionary *dict = &DefaultSMPTEDict();
    ASDCP::MXF::AS02_MCAConfigParser mca_config(dict);
    UL channel_assignment;

    RationalNumber frameRate = boost::any_cast<RationalNumber>(_muxerOptions["framerate"]);

    // set up essence parser
    Kumu::PathList_t fileList;
    fileList.push_back(file);
    Result_t result = Parser.OpenRead(fileList, Rational(frameRate.num, frameRate.denum));;

    // set up MXF writer
    if (ASDCP_SUCCESS(result)) {
        ASDCP::PCM::AudioDescriptor ADesc;
        Parser.FillAudioDescriptor(ADesc);

        ADesc.EditRate = Rational(frameRate.num, frameRate.denum);
        FrameBuffer.Capacity(PCM::CalcFrameBufferSize(ADesc));

        char buf[64];
        std::cout << ADesc.AudioSamplingRate.Quotient() / 1000.0 << "kHz PCM Audio, "
                      << frameRate.num << "x" << frameRate.denum << "fps " << PCM::CalcSamplesPerFrame(ADesc) << "spf" << std::endl;
        PCM::AudioDescriptorDump(ADesc);

        essence_descriptor = new ASDCP::MXF::WaveAudioDescriptor(dict);

        result = ASDCP::PCM_ADesc_to_MD(ADesc, essence_descriptor);

        if (mca_config.empty()) {
            essence_descriptor->ChannelAssignment = channel_assignment;
        } else {
            if (mca_config.ChannelCount() != essence_descriptor->ChannelCount) {
                fprintf(stderr, "MCA label count (%d) differs from essence stream channel count (%d).\n",
                                mca_config.ChannelCount(), essence_descriptor->ChannelCount);
                result = RESULT_FAIL;
            }

            // this is the d-cinema MCA label, what is the one for IMF?
            essence_descriptor->ChannelAssignment = dict->ul(MDD_DCAudioChannelCfg_MCA);
        }
    }

    if (ASDCP_SUCCESS(result)) {
        WriterInfo Info = s_MyInfo;  // fill in your favorite identifiers here
        Info.LabelSetType = LS_MXF_SMPTE;

        std::map<std::string, boost::any>::iterator it = _muxerOptions.find("asset_id");
        if (it != _muxerOptions.end()) {
            std::string assetId = boost::any_cast<std::string>(it->second);
            memcpy(Info.AssetUUID, assetId.c_str(), UUIDlen);
        } else {
            Kumu::GenRandomUUID(Info.AssetUUID);
        }

        // configure encryption
        /*
        if( Options.key_flag ) {
            Kumu::GenRandomUUID(Info.ContextID);
            Info.EncryptedEssence = true;

            if ( Options.key_id_flag )
                memcpy(Info.CryptographicKeyID, Options.key_id_value, UUIDlen);
            else
                RNG.FillRandom(Info.CryptographicKeyID, UUIDlen);

            Context = new AESEncContext;
            result = Context->InitKey(Options.key_value);

            if ( ASDCP_SUCCESS(result) )
                result = Context->SetIVec(RNG.FillRandom(IV_buf, CBC_BLOCK_SIZE));

            if ( ASDCP_SUCCESS(result) && Options.write_hmac ) {
                Info.UsesHMAC = true;
                HMAC = new HMACContext;
                result = HMAC->InitKey(Options.key_value, Info.LabelSetType);
            }
        }*/

        if (ASDCP_SUCCESS(result)) {
            result = Writer.OpenWrite(finalFile.c_str(), Info, essence_descriptor, mca_config, Rational(frameRate.num, frameRate.denum));
        }
    }

    if (ASDCP_SUCCESS(result)) {
        result = Parser.Reset();
        ui32_t duration = 0;

        while (ASDCP_SUCCESS(result)) {
            result = Parser.ReadFrame(FrameBuffer);

            if (ASDCP_SUCCESS(result)) {
                if (FrameBuffer.Size() != FrameBuffer.Capacity()) {
                    fprintf(stderr, "WARNING: Last frame read was short, PCM input is possibly not frame aligned.\n");
                    fprintf(stderr, "Expecting %u bytes, got %u.\n", FrameBuffer.Capacity(), FrameBuffer.Size());
                    result = RESULT_ENDOFFILE;
                    continue;
                }


                result = Writer.WriteFrame(FrameBuffer, Context, HMAC);

                // The Writer class will forward the last block of ciphertext
                // to the encryption context for use as the IV for the next
                // frame. If you want to use non-sequitur IV values, un-comment
                // the following  line of code.
                // if ( ASDCP_SUCCESS(result) && Options.key_flag )
                //   Context->SetIVec(RNG.FillRandom(IV_buf, CBC_BLOCK_SIZE));
            }
        }

        if (result == RESULT_ENDOFFILE) {
            result = RESULT_OK;
        }
    }

    if (ASDCP_SUCCESS(result)) {
        result = Writer.Finalize();
        if (ASDCP_FAILURE(result)) {
            throw std::runtime_error("[MXFWriter] Error writing mxf file");
        }
    }
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
        throw std::runtime_error("[MXFWriter] error opening j2k parser");
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

    int profileInt = boost::any_cast<int>(_muxerOptions["broadcast_profile"]);

    // 353 is profile 1.
    MDD_t profile = static_cast<MDD_t>(static_cast<int>(MDD_JP2KEssenceCompression_BroadcastProfile_1) + (profileInt - 1));

    if (yuvEssence) {
        MXF::CDCIEssenceDescriptor* cdciDescriptor = new MXF::CDCIEssenceDescriptor(dict);
        essenceSubDescriptors.push_back(pictureSubDescriptor);

        result = JP2K_PDesc_to_MD(pictureDescriptor,
                                  *dict,
                                  *static_cast<MXF::GenericPictureEssenceDescriptor*>(cdciDescriptor),
                                  *static_cast<MXF::JPEG2000PictureSubDescriptor*>(essenceSubDescriptors.back()));

        if (ASDCP_SUCCESS(result)) {
            cdciDescriptor->PictureEssenceCoding = UL(dict->ul(profile));
            cdciDescriptor->HorizontalSubsampling = boost::any_cast<int>(_muxerOptions["subsampling_dx"]);
            cdciDescriptor->VerticalSubsampling = boost::any_cast<int>(_muxerOptions["subsampling_dy"]);
            cdciDescriptor->ComponentDepth = boost::any_cast<int>(_muxerOptions["bits"]);
            cdciDescriptor->FrameLayout = 0;    // no interlaced shit
            cdciDescriptor->AspectRatio = pictureDescriptor.AspectRatio;
            cdciDescriptor->FieldDominance = 0; // only for interlaced shit // field dominance = 1 -> upper field first
            essenceDescriptor = static_cast<MXF::FileDescriptor*>(cdciDescriptor);
	}
    } else {
        MXF::RGBAEssenceDescriptor* rgbDescriptor = new MXF::RGBAEssenceDescriptor(dict);
        essenceSubDescriptors.push_back(pictureSubDescriptor);

        result = JP2K_PDesc_to_MD(pictureDescriptor,
                                  *dict,
                                  *static_cast<MXF::GenericPictureEssenceDescriptor*>(rgbDescriptor),
                                  *static_cast<MXF::JPEG2000PictureSubDescriptor*>(essenceSubDescriptors.back()));

        if (ASDCP_SUCCESS(result)) {
            rgbDescriptor->PictureEssenceCoding = UL(dict->ul(profile));
            rgbDescriptor->ComponentMaxRef = 1023;          // this must be 255 for 8 bit right?
            rgbDescriptor->ComponentMinRef = 0;
            essenceDescriptor = static_cast<MXF::FileDescriptor*>(rgbDescriptor);
	}
    }

    if (essenceDescriptor == nullptr) {
        throw std::runtime_error("[MXFWriter] Unable to obtain essence descriptor");
    }

    WriterInfo info = s_MyInfo;
    info.LabelSetType = LS_MXF_SMPTE;

    std::map<std::string, boost::any>::iterator it = _muxerOptions.find("asset_id");
    if (it != _muxerOptions.end()) {
        std::string assetId = boost::any_cast<std::string>(it->second);
        memcpy(info.AssetUUID, assetId.c_str(), UUIDlen);
    } else {
        Kumu::GenRandomUUID(info.AssetUUID);
    }

    //char strBuf[40];
    //std::cout << "ASSET ID: " << UUID(info.AssetUUID).EncodeHex(strBuf, 40) << std::endl;

    result = writer.OpenWrite(finalFile,
                              info,
                              essenceDescriptor,
                              essenceSubDescriptors,
                              pictureDescriptor.EditRate,
                              16384, // mxf header size
                              AS_02::IS_FOLLOW, // index strategy
                              60); // partition space

    if (ASDCP_FAILURE(result)) {
        throw std::runtime_error("[MXFWriter] Error opening mxf writer");
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
            throw std::runtime_error("[MXFWriter] Error writing mxf file");
        }
    }
}
