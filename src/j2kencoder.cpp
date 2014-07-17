#include "j2kencoder.h"
#include "rawvideoframe.h"

#include "j2kframe.h"

#include <stdexcept>
#include <iostream>
#include <cstring> // memset

#include <functional>

J2KEncoder::J2KEncoder(COLOR_FORMAT targetColorFormat, BIT_RATE targetBitRate, PROFILE profile, bool useTiles)
    : _targetColorFormat(targetColorFormat), _targetBitRate(targetBitRate), _profile(profile), _useTiles(useTiles)
{
    //ctor
}

J2KEncoder::~J2KEncoder()
{
    //dtor
}

void J2KEncoder::EncodeRawFrame(const RawVideoFrame &rawFrame, J2kFrame &encodedFrame)
{
    OPJ_COLOR_SPACE colorSpace;
    bool doMct = false;

    switch (_targetColorFormat) {
        case COLOR_FORMAT::CF_YUV422:
            throw std::runtime_error("j2k yuv 422 not possible yet");
        case COLOR_FORMAT::CF_YUV444:
            doMct = true;
            colorSpace = OPJ_COLOR_SPACE::OPJ_CLRSPC_SYCC;
            break;
        case COLOR_FORMAT::CF_RGB444:
            doMct = false;
            colorSpace = OPJ_COLOR_SPACE::OPJ_CLRSPC_SRGB;
            break;
        default:
            throw std::runtime_error("unsupported colorspace");
    }

    opj_cparameters_t encodingParameters;
    opj_set_default_encoder_parameters(&encodingParameters);
    encodingParameters.subsampling_dx = 1;
    encodingParameters.subsampling_dy = 1;
    encodingParameters.tcp_mct = (doMct ? 1 : 0);         // 0 = store as rgb, 1 = store as yuv ??? I THINK!!! :-)
    std::cout << "[J2K] " << (encodingParameters.tcp_mct == 0 ? "Store as RGB" : "Store as YUV") << std::endl;

    int widthUsed = rawFrame.width;
    int heightUsed = rawFrame.height;

    SetBroadcastProfile(encodingParameters, _profile, widthUsed, heightUsed);

    // will probably always be 3
    int numberComponents = 3;

    int rawBitDepth = static_cast<int>(_targetBitRate);

    std::cout << "[J2K] BitsPerComponent: " << rawBitDepth << std::endl;

    // no idea
    // int rawSigned = true;

    // ??? seriously ???
    int bigEndian = false;


    opj_image_cmptparm_t *componentParameter = (opj_image_cmptparm_t*) malloc((size_t)numberComponents * sizeof(opj_image_cmptparm_t));
    memset(&componentParameter[0], 0, (size_t)numberComponents * sizeof(opj_image_cmptparm_t));

    for (int i = 0; i < numberComponents; ++i) {
        componentParameter[i].prec = (uint32_t) rawBitDepth;
        componentParameter[i].bpp = (uint32_t) rawBitDepth;
        componentParameter[i].sgnd = (uint32_t) bigEndian;
        componentParameter[i].dx = (uint32_t) encodingParameters.subsampling_dx;// * (i > 0 ? subsamplingDx : 1);
        componentParameter[i].dy = (uint32_t) encodingParameters.subsampling_dy;// * (i > 0 ? subsamplingDy : 1);
        componentParameter[i].w = (uint32_t) widthUsed;
        componentParameter[i].h = (uint32_t) heightUsed;
    }

    // create image. NOTE: THIS IMAGE MUST BE FREE'D!!!
    opj_image_t *image = opj_image_create((uint32_t) numberComponents, &componentParameter[0], colorSpace);
    free(componentParameter);
    if(!image) {
        throw std::runtime_error("error creating jpeg2k image");
    }

    image->x0 = (uint32_t) encodingParameters.image_offset_x0;
    image->y0 = (uint32_t) encodingParameters.image_offset_y0;
    image->x1 = (uint32_t) encodingParameters.image_offset_x0 + (uint32_t) (widthUsed - 1) * (uint32_t) encodingParameters.subsampling_dx + 1;
    image->y1 = (uint32_t) encodingParameters.image_offset_y0 + (uint32_t) (heightUsed - 1) * (uint32_t) encodingParameters.subsampling_dy + 1;

    unsigned char* dataPtr = rawFrame.videoData[0];

    int jpegIndex = 0;
    for (int y = 0; y < heightUsed; ++y) {
        for (int x = 0; x < widthUsed; ++x) {
            /*if (rawBitDepth == 8) {
                int rgbIndex = x * 3;
                image->comps[0].data[jpegIndex] = dataPtr[y * 3 * widthUsed + (rgbIndex + 0)];	// R
                image->comps[1].data[jpegIndex] = dataPtr[y * 3 * widthUsed + (rgbIndex + 1)];	// G
                image->comps[2].data[jpegIndex] = dataPtr[y * 3 * widthUsed + (rgbIndex + 2)];	// B
            if (rawBitDepth <= 16) {*/
                int rgbIndex = x * 6;

                unsigned char r1 = dataPtr[y * 6 * widthUsed + (rgbIndex + 0)];
                unsigned char r2 = dataPtr[y * 6 * widthUsed + (rgbIndex + 1)];
                unsigned char g1 = dataPtr[y * 6 * widthUsed + (rgbIndex + 2)];
                unsigned char g2 = dataPtr[y * 6 * widthUsed + (rgbIndex + 3)];
                unsigned char b1 = dataPtr[y * 6 * widthUsed + (rgbIndex + 4)];
                unsigned char b2 = dataPtr[y * 6 * widthUsed + (rgbIndex + 5)];
                unsigned short red;
                unsigned short green;
                unsigned short blue;
                if(bigEndian) {
                    red = (unsigned short)((r1 << 8) + r2);
                    green = (unsigned short)((g1 << 8) + g2);
                    blue = (unsigned short)((b1 << 8) + b2);
                } else {
                    red = (unsigned short)((r2 << 8) + r1);
                    green = (unsigned short)((g2 << 8) + g1);
                    blue = (unsigned short)((b2 << 8) + b1);
                }
                image->comps[0].data[jpegIndex] = (unsigned short) red;;	// R
                image->comps[1].data[jpegIndex] = (unsigned short) green;;//dataPtr[y * 3 * widthUsed + (rgbIndex + 1)];	// G
                image->comps[2].data[jpegIndex] = (unsigned short) blue;//dataPtr[y * 3 * widthUsed + (rgbIndex + 2)];	// B
            //}
            jpegIndex++;
        }
    }

    bool success = EncodeImage(image, encodedFrame, encodingParameters, widthUsed, heightUsed);

    opj_image_destroy(image);

    // this gets allocated in openjpeg lib. its seriously ridicolous :D nothing mentioned
    // in the api or anywhere. nevertheless, we have to free it here
    if (encodingParameters.cp_comment != nullptr) {
        free(encodingParameters.cp_comment);
    }

    if (success == false) {
        throw std::runtime_error("error encoding image");
    }
}

bool J2KEncoder::EncodeImage(opj_image_t *image, J2kFrame &encodedFrame, opj_cparameters_t &parameters, int widthUsed, int heightUsed)
{

    opj_codec_t* codec = opj_create_compress((OPJ_CODEC_FORMAT) 0);
    if (codec == nullptr) {
        std::cout << "error creating j2k codec" << std::endl;
        return false;
    }

    opj_set_info_handler(codec, [](const char* s, void *) { std::cout << "[J2K INFO] " << s; }, nullptr);
    opj_set_warning_handler(codec, [](const char* s, void *) { std::cout << "[J2K WARNING] " << s; }, nullptr);
    opj_set_error_handler(codec, [](const char* s, void *) { std::cout << "[J2K ERROR] " << s; }, nullptr);

    opj_setup_encoder(codec, &parameters, image);

    opj_stream_t* stream = opj_stream_default_create(false);
    if (stream == nullptr) {
        opj_destroy_codec(codec);
        return false;
    }

    opj_stream_set_user_data(stream, &encodedFrame, nullptr);
    opj_stream_set_write_function(stream, &J2KEncoder::WriteJ2kFrame);

    bool success = opj_start_compress(codec, image, stream);
    if (success == false)  {
        std::cout << "ERROR opj_start_compress" << std::endl;
    } else {
        /* this routine doesnt work. seems to be experimental
        if (false && _useTiles && (_profile == PROFILE::BCP_MT_6 || _profile == PROFILE::BCP_MT_7)) {
            std::cout << "[J2K] Write Tiles" << std::endl;
            OPJ_BYTE *data;
            OPJ_UINT32 dataSize = (widthUsed / 2) * (heightUsed / 2) * 3;
            data = (OPJ_BYTE*) malloc( dataSize * sizeof(OPJ_BYTE));
            memset(data, 0, dataSize );
            for (int i = 0; i < 4; ++i) {
                success = opj_write_tile(codec, i, data, dataSize, stream);
                if (success == false) {
                    std::cout << "ERROR opj_write_tile" << std::endl;
                    break;
                }
            }
            free(data);
        } else {
        */
            success = success && opj_encode(codec, stream);
            if (success == false) {
                std::cout << "ERROR opj_encode" << std::endl;
            }
        //}
    }
    success = success && opj_end_compress(codec, stream);
    if (success == false)  {
        std::cout << "failed to encode image: opj_end_compress" << std::endl;
    }

    opj_stream_destroy(stream);
    opj_destroy_codec(codec);

    return success;
}

OPJ_SIZE_T J2KEncoder::WriteJ2kFrame(void *data, OPJ_SIZE_T bufferSize, void *userData)
{
    J2kFrame *frame = static_cast<J2kFrame*>(userData);
    frame->data.reserve(bufferSize);
    std::copy((uint8_t*)data, (uint8_t*)data + bufferSize, std::back_inserter(frame->data));

    return bufferSize;
}

void J2KEncoder::SetBroadcastProfile(opj_cparameters_t &encodingParameters, PROFILE profile, int widthUsed, int heightUsed)
{
    // set the ones that are the same for all profiles
    encodingParameters.roi_compno = -1;         // no region of interest
    encodingParameters.mode = 0;                // code block style 0000 0000
    encodingParameters.cblockw_init = 128;        // in asdcplib: 32 = 3, 128 = 5, in obj_dump: 128=2^7 , correct
    encodingParameters.cblockh_init = 128;
    encodingParameters.prog_order = OPJ_CPRL;   // progression order CPRL, is 4
    encodingParameters.numpocs = 0;             // no poc marker

    encodingParameters.cp_tx0 = 0;
    encodingParameters.cp_ty0 = 0;

    encodingParameters.tp_flag = 'C';           // one tile part for each component
    encodingParameters.tp_on = 1;               // generate tile part

    // one layer only
    encodingParameters.tcp_rates[0] = 0;	// MOD antonin : losslessbug
    encodingParameters.tcp_numlayers = 1;
    encodingParameters.cp_disto_alloc = 1;

    // use custom precinct size
    encodingParameters.csty |= 0x01;
    // this will cut precinct size of lowest band LL
    encodingParameters.res_spec = encodingParameters.numresolution - 1;
    for (int i = 0; i < encodingParameters.numresolution; ++i) {
        // the standard says precinct size 8 for any band > LL
        // but here we have to put in 2^8
        encodingParameters.prcw_init[i] = 256;
        encodingParameters.prch_init[i] = 256;
    }

    // To-DO: Set rates

    switch (profile) {
        case PROFILE::BCP_ST_1:
            // SetRate(
        case PROFILE::BCP_ST_2:
        case PROFILE::BCP_ST_3:
        case PROFILE::BCP_ST_4:
        case PROFILE::BCP_ST_5:
            if (_useTiles) {
                throw std::runtime_error("single tile broadcast profile cant be used with tiles");
            }
            encodingParameters.tile_size_on = OPJ_FALSE;
            encodingParameters.cp_tdx = 1;              // one huge tile
            encodingParameters.cp_tdy = 1;
            encodingParameters.irreversible = 1;        // 9-7 Irreversible Transform
            break;
        case PROFILE::BCP_MT_6:
        case PROFILE::BCP_MT_7:
            if (_useTiles) {
                encodingParameters.tile_size_on = OPJ_TRUE;
                encodingParameters.cp_tdx = widthUsed / 2;          // 4 tiles
                encodingParameters.cp_tdy = heightUsed / 2;
            } else {
                encodingParameters.tile_size_on = OPJ_FALSE;
                encodingParameters.cp_tdx = 1;
                encodingParameters.cp_tdy = 1;
            }
            encodingParameters.irreversible = 0;        // 5-3 Reversible Transform
            //throw std::runtime_error("multi tiles not implemted yet");
            break;
        default:
            throw std::runtime_error("undefined broadcast profile");

    }
}
