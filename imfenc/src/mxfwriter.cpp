#include "mxfwriter.h"

#include <KM_fileio.h>
#include <KM_prng.h>
#include <AS_02.h>
#include <Metadata.h>
#include <PCMParserList.h>
#include <iostream>

#include <stdexcept>
#include <memory>
#include <cmath>

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

MXFWriter::MXFWriter()
{
    //ctor
}

MXFWriter::~MXFWriter()
{
    //dtor
}

void MXFWriter::MuxAudioFile(const std::string& file, const std::string &finalFile, const MXFOptionsAudio& options)
{
    AESEncContext* Context = 0;
    HMACContext* HMAC = 0;
    PCMParserList Parser;
    AS_02::PCM::MXFWriter Writer;
    PCM::FrameBuffer FrameBuffer;
    Kumu::FortunaRNG RNG;
    ASDCP::MXF::WaveAudioDescriptor *essence_descriptor = 0;
    const Dictionary *dict = &DefaultSMPTEDict();
    ASDCP::MXF::AS02_MCAConfigParser mca_config(dict);
    UL channel_assignment;

    // set up essence parser
    Kumu::PathList_t fileList;
    fileList.push_back(file);
    Result_t result = Parser.OpenRead(fileList, Rational(options.editRate.num, options.editRate.denum));;

    // set up MXF writer
    if (ASDCP_SUCCESS(result)) {
        ASDCP::PCM::AudioDescriptor ADesc;
        Parser.FillAudioDescriptor(ADesc);

        ADesc.EditRate = Rational(options.editRate.num, options.editRate.denum);
        FrameBuffer.Capacity(PCM::CalcFrameBufferSize(ADesc));

        std::cout << ADesc.AudioSamplingRate.Quotient() / 1000.0 << "kHz PCM Audio, "
                      << options.editRate.num << "x" << options.editRate.denum << "fps " << PCM::CalcSamplesPerFrame(ADesc) << "spf" << std::endl;
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

        Kumu::GenRandomUUID(Info.AssetUUID);

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
            result = Writer.OpenWrite(finalFile.c_str(), Info, essence_descriptor, mca_config, Rational(options.editRate.num, options.editRate.denum));
        }
    }

    if (ASDCP_SUCCESS(result)) {
        result = Parser.Reset();

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

void MXFWriter::MuxVideoFiles(const std::list<std::string> &files, const std::string& finalFile, const MXFOptionsVideo &options)
{
    AESEncContext* context = nullptr;
    HMACContext* HMAC = nullptr;
    AS_02::JP2K::MXFWriter writer;
    JP2K::FrameBuffer frameBuffer(12 * Kumu::Megabyte); // enough?
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

    RationalNumber aspectRatio = options.aspectRatio;
    RationalNumber frameRate = options.editRate;

    JP2K::PictureDescriptor pictureDescriptor;
    parser.FillPictureDescriptor(pictureDescriptor);
    pictureDescriptor.EditRate = Rational(frameRate.num, frameRate.denum);
    pictureDescriptor.SampleRate = Rational(frameRate.num, frameRate.denum);
    pictureDescriptor.ContainerDuration = options.containerDuration;
    pictureDescriptor.AspectRatio = Rational(aspectRatio.num, aspectRatio.denum);

    JP2K::PictureDescriptorDump(pictureDescriptor);

    MXF::JPEG2000PictureSubDescriptor* pictureSubDescriptor = new MXF::JPEG2000PictureSubDescriptor(dict);

    bool yuvEssence = options.yuvEssence;

    int profileInt = options.broadcastProfile;

    // 353 is profile 1.
    MDD_t profile = static_cast<MDD_t>(static_cast<int>(MDD_JP2KEssenceCompression_BroadcastProfile_1) + (profileInt - 1));
    UL transferCharacteristic;
    UL colorPrimaries;
    UL codingEquations;
    // BT2020
    if (options.fullRange) {
        transferCharacteristic.DecodeHex("060e2b34.0401010e.04010101.01090000");
        colorPrimaries.DecodeHex("060e2b34.0401010d.04010101.03040000");
        codingEquations.DecodeHex("060e2b34.0401010d.04010101.02060000");

    // BT709
    } else {
        transferCharacteristic.DecodeHex("060e2b34.04010101.04010101.01020000");
        colorPrimaries.DecodeHex("060e2b34.04010106.04010101.03030000");
        codingEquations.DecodeHex("060e2b34.04010101.04010101.02020000");
    }

    if (yuvEssence) {
        MXF::CDCIEssenceDescriptor* cdciDescriptor = new MXF::CDCIEssenceDescriptor(dict);
        essenceSubDescriptors.push_back(pictureSubDescriptor);

        result = JP2K_PDesc_to_MD(pictureDescriptor,
                                  *dict,
                                  *static_cast<MXF::GenericPictureEssenceDescriptor*>(cdciDescriptor),
                                  *static_cast<MXF::JPEG2000PictureSubDescriptor*>(essenceSubDescriptors.back()));

        if (ASDCP_SUCCESS(result)) {
            switch (options.bits) {
                case 8:
                    pictureSubDescriptor->J2CLayout.set(ASDCP::MXF::RGBAValue_YUV_8);
                    break;
                case 10:
                case 12: // hmm ... nothing defined for 12 bit
                default:
                    pictureSubDescriptor->J2CLayout.set(ASDCP::MXF::RGBAValue_YUV_10);
                    break;
            }

            cdciDescriptor->PictureEssenceCoding = UL(dict->ul(profile));
            cdciDescriptor->HorizontalSubsampling = options.subsamplingDx;
            cdciDescriptor->VerticalSubsampling = 1; // no vertical subsampling in IMF
            cdciDescriptor->ComponentDepth = options.bits;
            cdciDescriptor->FrameLayout = 0;    // no interlaced shit
            cdciDescriptor->AspectRatio = pictureDescriptor.AspectRatio;
            cdciDescriptor->FieldDominance = 0; // only for interlaced shit // field dominance = 1 -> upper field first
            cdciDescriptor->WhiteReflevel = 940;
            cdciDescriptor->BlackRefLevel = 64;
            cdciDescriptor->PictureEssenceCoding = UL(dict->ul(profile));
            cdciDescriptor->TransferCharacteristic = transferCharacteristic;
            cdciDescriptor->ColorPrimaries = colorPrimaries;
            cdciDescriptor->CodingEquations = codingEquations;

            essenceDescriptor = static_cast<MXF::FileDescriptor*>(cdciDescriptor);

            cdciDescriptor->Dump();
	    }
    } else {
        MXF::RGBAEssenceDescriptor* rgbDescriptor = new MXF::RGBAEssenceDescriptor(dict);
        essenceSubDescriptors.push_back(pictureSubDescriptor);

        result = JP2K_PDesc_to_MD(pictureDescriptor,
                                  *dict,
                                  *static_cast<MXF::GenericPictureEssenceDescriptor*>(rgbDescriptor),
                                  *static_cast<MXF::JPEG2000PictureSubDescriptor*>(essenceSubDescriptors.back()));

        if (ASDCP_SUCCESS(result)) {

            switch (options.bits) {
                case 8:
                    pictureSubDescriptor->J2CLayout.set(ASDCP::MXF::RGBAValue_RGB_8);
                    break;
                case 10:
                case 12: // hmm ... nothing defined for 12 bit
                default:
                    pictureSubDescriptor->J2CLayout.set(ASDCP::MXF::RGBAValue_RGB_10);
                    break;
            }

            rgbDescriptor->PictureEssenceCoding = UL(dict->ul(profile));
            rgbDescriptor->TransferCharacteristic = transferCharacteristic;
            rgbDescriptor->ColorPrimaries = colorPrimaries;
            rgbDescriptor->CodingEquations = codingEquations;

            // BT2020
            if (options.fullRange) {
                rgbDescriptor->ComponentMaxRef = std::pow(2, options.bits) - 1;
                rgbDescriptor->ComponentMinRef = 0;

            // BT709
            } else {
                const int upperConstraint[3] = {235, 940, 3760};
                const int lowerConstraint[3] = {16, 64, 256};

                switch (options.bits) {
                    case 8:
                        rgbDescriptor->ComponentMaxRef = upperConstraint[0];
                        rgbDescriptor->ComponentMinRef = lowerConstraint[0];
                        break;
                    case 10:
                        rgbDescriptor->ComponentMaxRef = upperConstraint[1];
                        rgbDescriptor->ComponentMinRef = lowerConstraint[1];
                        break;
                    case 12:
                        rgbDescriptor->ComponentMaxRef = upperConstraint[2];
                        rgbDescriptor->ComponentMinRef = lowerConstraint[2];
                        break;
                    default:
                        // should never go here. fallback to full range
                        rgbDescriptor->ComponentMaxRef = std::pow(2, options.bits) - 1;
                        rgbDescriptor->ComponentMinRef = 0;
                        break;
                }
            }

            /* TO-DO: figure out what this means and if its necessary or if this code can go
            if (Options.md_min_luminance || Options.md_max_luminance) {
                rgbDescriptor->MasteringDisplayMinimumLuminance = Options.md_min_luminance;
                rgbDescriptor->MasteringDisplayMaximumLuminance = Options.md_max_luminance;
            }
            if (Options.md_primaries.HasValue()) {
                rgbDescriptor->MasteringDisplayPrimaries = Options.md_primaries;
                rgbDescriptor->MasteringDisplayWhitePointChromaticity = Options.md_white_point;
            }
            */

            essenceDescriptor = static_cast<MXF::FileDescriptor*>(rgbDescriptor);
            rgbDescriptor->Dump();
	    }
    }

    if (essenceDescriptor == nullptr) {
        throw std::runtime_error("[MXFWriter] Unable to obtain essence descriptor");
    }

    WriterInfo info = s_MyInfo;
    info.LabelSetType = LS_MXF_SMPTE;

    Kumu::GenRandomUUID(info.AssetUUID);

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
