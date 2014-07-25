#include "j2kencoder.h"
#include "common.h"


#include <stdexcept>
#include <iostream>
#include <cstring> // memset

#include <functional>
#include <cmath>

static inline float u8tofloat_trick2(uint8_t x)
{
    union { float f; uint32_t i; } u; u.f = 32768.0f; u.i |= x;
    return (u.f - 32768.0f) * (256.0f / 255.0f);
}

static inline uint8_t u8fromfloat_trick(float x)
{
    union { float f; uint32_t i; } u;
    u.f = 32768.0f + x * (255.0f / 256.0f);
    return (uint8_t)u.i;
}

J2KEncoder::J2KEncoder(COLOR_FORMAT targetColorFormat, BIT_RATE targetBitRate, PROFILE profile, bool useTiles, RationalNumber fps, int width, int height)
    :
    _targetColorFormat(targetColorFormat), _targetBitRate(targetBitRate), _profile(profile), _useTiles(useTiles), _fps(fps),
    _componentParameter(nullptr), _widthUsed(width), _heightUsed(height), _encodingParameters(), _bigEndian(false),
    _colorSpace(_targetColorFormat == CF_RGB444 ? OPJ_CLRSPC_SRGB : OPJ_CLRSPC_SRGB)
{
    _encodingParameters.cp_comment = nullptr;
}

J2KEncoder::~J2KEncoder()
{
    if (_componentParameter != nullptr) {
        free(_componentParameter);
    }
    if (_encodingParameters.cp_comment != nullptr) {
        free(_encodingParameters.cp_comment);
    }
}

void J2KEncoder::InitEncoder()
{
    if (_widthUsed <= 0 || _heightUsed <= 0) {
        throw std::runtime_error("[J2K ERROR] Width or Height must be bigger than 0");
    }
    if (_fps.num == 0) {
        throw std::runtime_error("[J2K ERROR] Invalid framerate");
    }

    // will probably always be 3
    int numberComponents = 3;

    opj_set_default_encoder_parameters(&_encodingParameters);
    SetBroadcastProfile();
    SetRates(3 * _widthUsed * _heightUsed * static_cast<int>(_targetBitRate));

    _componentParameter = (opj_image_cmptparm_t*) malloc((size_t)numberComponents * sizeof(opj_image_cmptparm_t));
    memset(&_componentParameter[0], 0, (size_t)numberComponents * sizeof(opj_image_cmptparm_t));

    for (int i = 0; i < numberComponents; ++i) {
        _componentParameter[i].prec = (uint32_t) _targetBitRate;
        _componentParameter[i].bpp = (uint32_t) _targetBitRate;
        _componentParameter[i].sgnd = (uint32_t) _bigEndian;
        _componentParameter[i].dx = (uint32_t) _encodingParameters.subsampling_dx;// * (i > 0 ? subsamplingDx : 1);
        _componentParameter[i].dy = (uint32_t) _encodingParameters.subsampling_dy;// * (i > 0 ? subsamplingDy : 1);
        _componentParameter[i].w = (uint32_t) _widthUsed;
        _componentParameter[i].h = (uint32_t) _heightUsed;
    }

    std::cout << "[J2K] BitsPerComponent: " << (int)_targetBitRate << std::endl;
}

void J2KEncoder::EncodeRawFrame(const RawVideoFrame &rawFrame, J2kFrame &encodedFrame)
{
    // create image. NOTE: THIS IMAGE MUST BE FREE'D!!!
    opj_image_t *image = opj_image_create(3, &_componentParameter[0], _colorSpace);
    if (image == nullptr) {
        throw std::runtime_error("error creating jpeg2k image");
    }

    image->x0 = (uint32_t) _encodingParameters.image_offset_x0;
    image->y0 = (uint32_t) _encodingParameters.image_offset_y0;
    image->x1 = (uint32_t) _encodingParameters.image_offset_x0 + (uint32_t) (_widthUsed - 1) * (uint32_t) _encodingParameters.subsampling_dx + 1;
    image->y1 = (uint32_t) _encodingParameters.image_offset_y0 + (uint32_t) (_heightUsed - 1) * (uint32_t) _encodingParameters.subsampling_dy + 1;

    unsigned char* dataPtr = rawFrame.videoData[0];

    int rawBitDepth = (int) _targetBitRate;

    int jpegIndex = 0;
    for (int y = 0; y < _heightUsed; ++y) {
        for (int x = 0; x < _widthUsed; ++x) {
            if (rawBitDepth == 8) {
                int rgbIndex = x * 3;
                ColorComponent colors;
                colors.c1 = dataPtr[y * 3 * _widthUsed + (rgbIndex + 0)];	// R
                colors.c2 = dataPtr[y * 3 * _widthUsed + (rgbIndex + 1)];	// G
                colors.c3 = dataPtr[y * 3 * _widthUsed + (rgbIndex + 2)];	// B

                if (_targetColorFormat != CF_RGB444) {
                    RGB24toYUV24(colors);
                }

                image->comps[0].data[jpegIndex] = colors.c1;
                image->comps[1].data[jpegIndex] = colors.c2;
                image->comps[2].data[jpegIndex] = colors.c3;

            } else if (rawBitDepth == 10 || rawBitDepth == 12) {
                int rgbIndex = x * 3;
                ColorComponent colors;
                colors.c1 = dataPtr[y * 3 * _widthUsed + (rgbIndex + 0)];	// R
                colors.c2 = dataPtr[y * 3 * _widthUsed + (rgbIndex + 1)];	// G
                colors.c3 = dataPtr[y * 3 * _widthUsed + (rgbIndex + 2)];	// B

                if (_targetColorFormat != CF_RGB444) {
                    RGB24toYUV24(colors);
                }

                unsigned short red = colors.c1 << (rawBitDepth - 8);
                unsigned short green = colors.c2 << (rawBitDepth - 8);
                unsigned short blue = colors.c3 << (rawBitDepth - 8);

                image->comps[0].data[jpegIndex] = (unsigned short) red;
                image->comps[1].data[jpegIndex] = (unsigned short) green;
                image->comps[2].data[jpegIndex] = (unsigned short) blue;
            }

            /* else if (rawBitDepth <= 16) {
                int rgbIndex = x * 6;

                unsigned char r1 = dataPtr[y * 6 * _widthUsed + (rgbIndex + 0)];
                unsigned char r2 = dataPtr[y * 6 * _widthUsed + (rgbIndex + 1)];
                unsigned char g1 = dataPtr[y * 6 * _widthUsed + (rgbIndex + 2)];
                unsigned char g2 = dataPtr[y * 6 * _widthUsed + (rgbIndex + 3)];
                unsigned char b1 = dataPtr[y * 6 * _widthUsed + (rgbIndex + 4)];
                unsigned char b2 = dataPtr[y * 6 * _widthUsed + (rgbIndex + 5)];
                unsigned short red;
                unsigned short green;
                unsigned short blue;
                if (_bigEndian) {
                    red = (unsigned short)((r1 << 8) + r2);
                    green = (unsigned short)((g1 << 8) + g2);
                    blue = (unsigned short)((b1 << 8) + b2);
                } else {
                    red = (unsigned short)((r2 << 8) + r1);
                    green = (unsigned short)((g2 << 8) + g1);
                    blue = (unsigned short)((b2 << 8) + b1);
                }
                image->comps[0].data[jpegIndex] = (unsigned short) red;
                image->comps[1].data[jpegIndex] = (unsigned short) green;
                image->comps[2].data[jpegIndex] = (unsigned short) blue;
            }*/
            jpegIndex++;
        }
    }

    bool success = EncodeImage(image, encodedFrame);

    opj_image_destroy(image);

    if (success == false) {
        throw std::runtime_error("error encoding image");
    }
}

void J2KEncoder::RGB24toYUV24(ColorComponent &rgb)
{
    unsigned char y, u, v;
    unsigned char r = rgb.c1;
    unsigned char g = rgb.c2;
    unsigned char b = rgb.c3;
    // FIXED POINT matrix
    //y = (( 66 * r + 129 * g + 25  * b) >> 8) + 16;
    //u = ((-38 * r - 74  * g + 112 * b) >> 8) + 128;
    //v = ((112 * r - 94  * g - 18  * b) >> 8) + 128;



    // JPEG matrix
    float fr = u8tofloat_trick2(r);
    float fg = u8tofloat_trick2(g);
    float fb = u8tofloat_trick2(b);
    y = u8fromfloat_trick(0   + (0.299f    * fr) + (0.587f    * fg) + (0.114f    * fb));
    u = u8fromfloat_trick(128 - (0.168736f * fr) - (0.331264f * fg) + (0.5f      * fb));
    v = u8fromfloat_trick(128 + (0.5f      * fr) - (0.418688f * fg) - (0.081312f * fb));
    //fy = (0.299f * fr + 0.587f * fg + 0.114f * fb);
    //y = u8fromfloat_trick(fy);
    //u = u8fromfloat_trick(0.492f * u8tofloat_trick2(b - y));
    //v = u8fromfloat_trick(0.877f * u8tofloat_trick2(b - y));

    rgb.c1 = y;
    rgb.c2 = u;
    rgb.c3 = v;
}

bool J2KEncoder::EncodeImage(opj_image_t *image, J2kFrame &encodedFrame)
{

    opj_codec_t* codec = opj_create_compress((OPJ_CODEC_FORMAT) 0);
    if (codec == nullptr) {
        std::cout << "error creating j2k codec" << std::endl;
        return false;
    }

    opj_set_info_handler(codec, [](const char* s, void *) { std::cout << "[J2K INFO] " << s; }, nullptr);
    opj_set_warning_handler(codec, [](const char* s, void *) { std::cout << "[J2K WARNING] " << s; }, nullptr);
    opj_set_error_handler(codec, [](const char* s, void *) { std::cout << "[J2K ERROR] " << s; }, nullptr);

    opj_setup_encoder(codec, &_encodingParameters, image);

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

void J2KEncoder::SetBroadcastProfile()
{
    _encodingParameters.cp_comment = (char*)malloc(32);
    strcpy(_encodingParameters.cp_comment, "ODMedia J2K");
    _encodingParameters.subsampling_dx = 1;
    _encodingParameters.subsampling_dy = 1;
    _encodingParameters.tcp_mct = 1;        // I really wonder what this does???

    // one layer only
    _encodingParameters.tcp_rates[0] = 0;	// MOD antonin : losslessbug
    _encodingParameters.tcp_numlayers = 1;
    _encodingParameters.cp_disto_alloc = 1;

    // set the ones that are the same for all profiles
    _encodingParameters.roi_compno = -1;         // no region of interest
    _encodingParameters.mode = 0;                // code block style 0000 0000

    _encodingParameters.cblockw_init = 64;        // 2^6
    _encodingParameters.cblockh_init = 64;

    _encodingParameters.prog_order = OPJ_CPRL;   // progression order CPRL, is 4
    _encodingParameters.numpocs = 0;             // no poc marker

    _encodingParameters.cp_tx0 = 0;
    _encodingParameters.cp_ty0 = 0;

    _encodingParameters.tp_flag = 'C';           // one tile part for each component
    _encodingParameters.tp_on = 1;               // generate tile part

    // use custom precinct size
    _encodingParameters.csty |= 0x01;
    // this will cut precinct size of lowest band LL
    _encodingParameters.res_spec = _encodingParameters.numresolution - 1;
    for (int i = 0; i < _encodingParameters.numresolution; ++i) {
        // the standard says precinct size 8 for any band > LL
        // but here we have to put in 2^8
        _encodingParameters.prcw_init[i] = 256;
        _encodingParameters.prch_init[i] = 256;
    }

    // To-DO: Set rates
    //"JPEG 2000 Profile-3 and 4 (2k/4k dc profile) requires:\n"
      //                "Maximum 1302083 compressed bytes @ 24fps\n"
        //              "-> Specified rate exceeds this limit. Rate will be forced to 1302083 bytes.\n");
        //parameters->max_cs_size = OPJ_CINEMA_24_CS;

    //"Maximum 1041666 compressed bytes @ 24fps\n"
      //                "-> Specified rate exceeds this limit. Rate will be forced to 1041666 bytes.\n");
        //parameters->max_comp_size = OPJ_CINEMA_24_COMP;
    //parameters->tcp_rates[0] = (OPJ_FLOAT32) (image->numcomps * image->comps[0].w * image->comps[0].h * image->comps[0].prec)/
            //(OPJ_FLOAT32)(((OPJ_UINT32)parameters->max_cs_size) * 8 * image->comps[0].dx * image->comps[0].dy);

    switch (_profile) {
        case PROFILE::BCP_ST_1:
            // SetRate(
        case PROFILE::BCP_ST_2:
        case PROFILE::BCP_ST_3:
        case PROFILE::BCP_ST_4:
        case PROFILE::BCP_ST_5:
            if (_useTiles) {
                throw std::runtime_error("single tile broadcast profile cant be used with tiles");
            }
            _encodingParameters.tile_size_on = OPJ_FALSE;
            _encodingParameters.cp_tdx = 1;              // one huge tile
            _encodingParameters.cp_tdy = 1;
            _encodingParameters.irreversible = 1;        // 9-7 Irreversible Transform
            break;
        case PROFILE::BCP_MT_6:
        case PROFILE::BCP_MT_7:
            if (_useTiles) {
                _encodingParameters.tile_size_on = OPJ_TRUE;
                _encodingParameters.cp_tdx = _widthUsed / 2;          // 4 tiles
                _encodingParameters.cp_tdy = _heightUsed / 2;
            } else {
                _encodingParameters.tile_size_on = OPJ_FALSE;
                _encodingParameters.cp_tdx = 1;
                _encodingParameters.cp_tdy = 1;
            }
            _encodingParameters.irreversible = 0;        // 5-3 Reversible Transform
            //throw std::runtime_error("multi tiles not implemted yet");
            break;
        default:
            throw std::runtime_error("undefined broadcast profile");

    }
}

void J2KEncoder::SetRates(int frameSize)
{
    const int profileIndex = static_cast<int>(_profile);
    // profile 7 has no limitation
    if (profileIndex < 7) {
        // only for profile 1 to 6, 7 is unspecfified
        const int MAX_COMP_BITRATE[6] = {200, 200, 200, 400, 800, 1600};

        //const int MAX_COMP_SAMPLING_RATE[7] = {65, 130, 195, 260, 520, 520, 520};

        const int MEGA = 1000000;       // 10 ^ 6

        // formula is target rate * 10^6 / ( targetFps * 8 )
        float fps = (float)_fps.num / (float)_fps.denum;
        _encodingParameters.max_comp_size = floorf((MAX_COMP_BITRATE[profileIndex - 1] / (fps * 8)) * MEGA);
        std::cout << "[J2K] max compressed bit rate for profile " << profileIndex << " with " << _fps.num << "/" << _fps.denum << " fps: " << _encodingParameters.max_comp_size << std::endl;

        // TO-DO: this is a heuristic. I have no idea how to calculate max_cs_size. When I know, this should be fixed
        _encodingParameters.max_cs_size = floorf(_encodingParameters.max_comp_size * 1.25f);
        if (_encodingParameters.max_cs_size > 0) {
            _encodingParameters.tcp_rates[0] = (OPJ_FLOAT32) frameSize / (OPJ_FLOAT32) (((OPJ_UINT32)_encodingParameters.max_cs_size) * 8);
        }
        std::cout << "[J2K] max_cs_size: " << _encodingParameters.max_cs_size << std::endl;
    } else {
        std::cout << "[J2K] lossless compression" << std::endl;
    }
}
