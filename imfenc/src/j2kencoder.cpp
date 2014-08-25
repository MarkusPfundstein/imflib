#include "j2kencoder.h"
#include "common.h"


#include <stdexcept>
#include <iostream>
#include <cstring> // memset

#include <functional>
#include <cmath>

J2KEncoder::J2KEncoder(BIT_RATE targetBitDepth, PROFILE profile, bool useTiles, RationalNumber fps, int width, int height, bool doMct, bool sc)
    :
    _targetBitDepth(targetBitDepth), _profile(profile), _useTiles(useTiles), _fps(fps),
    _componentParameter(nullptr), _widthUsed(width), _heightUsed(height), _encodingParameters(), _bigEndian(false),
    _colorSpace(OPJ_CLRSPC_SRGB), _mct(doMct), _subsampledChroma(sc)
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
        throw std::runtime_error("[J2K] Width or Height must be bigger than 0");
    }
    if (_fps.num == 0) {
        throw std::runtime_error("[J2K] Invalid framerate");
    }

    // will probably always be 3
    int numberComponents = 3;

    opj_set_default_encoder_parameters(&_encodingParameters);
    SetBroadcastProfile();
    SetRates(3 * _widthUsed * _heightUsed * static_cast<int>(_targetBitDepth));

    _componentParameter = (opj_image_cmptparm_t*) malloc((size_t)numberComponents * sizeof(opj_image_cmptparm_t));
    memset(&_componentParameter[0], 0, (size_t)numberComponents * sizeof(opj_image_cmptparm_t));

    for (int i = 0; i < numberComponents; ++i) {
        _componentParameter[i].prec = (uint32_t) _targetBitDepth;
        _componentParameter[i].bpp = (uint32_t) _targetBitDepth;
        _componentParameter[i].sgnd = (uint32_t) _bigEndian;
        _componentParameter[i].dx = (uint32_t) i == 0 ? 1 : _encodingParameters.subsampling_dx;
        _componentParameter[i].dy = (uint32_t) i == 0 ? 1 : _encodingParameters.subsampling_dy;
        _componentParameter[i].w = (uint32_t) _widthUsed;
        _componentParameter[i].h = (uint32_t) _heightUsed;
    }

    std::cout << "[J2K] BitsPerComponent: " << (int)_targetBitDepth << std::endl;
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
    image->x1 = (uint32_t) _encodingParameters.image_offset_x0 + (uint32_t) (_widthUsed);//((uint32_t) _encodingParameters.subsampling_dx + 1);
    image->y1 = (uint32_t) _encodingParameters.image_offset_y0 + (uint32_t) (_heightUsed);//(uint32_t) _encodingParameters.subsampling_dy + 1;

    if (rawFrame.planar) {
        FillImagePlanar(image, rawFrame);
    } else {
        FillImagePacked(image, rawFrame);
    }

    bool success = EncodeImage(image, encodedFrame);

    opj_image_destroy(image);

    if (success == false) {
        throw std::runtime_error("[J2K] Error encoding image");
    }
}

void J2KEncoder::FillImagePacked(opj_image_t *image, const RawVideoFrame &rawFrame)
{
    int jpegIndex = 0;
    unsigned char* dataPtr = (unsigned char*)&rawFrame.videoData[0].front();
    for (int y = 0; y < _heightUsed; ++y) {
        for (int x = 0; x < _widthUsed; ++x) {
            if (_targetBitDepth == BR_8bit) {
                int rgbIndex = x * 3;
                uint8_t c1 = dataPtr[y * 3 * _widthUsed + (rgbIndex + 0)];	// R
                uint8_t c2 = dataPtr[y * 3 * _widthUsed + (rgbIndex + 1)];	// G
                uint8_t c3 = dataPtr[y * 3 * _widthUsed + (rgbIndex + 2)];	// B

                image->comps[0].data[jpegIndex] = c1;
                image->comps[1].data[jpegIndex] = c2;
                image->comps[2].data[jpegIndex] = c3;
                jpegIndex++;
            }
        }
    }

}

void J2KEncoder::FillImagePlanar(opj_image_t *image, const RawVideoFrame &rawFrame)
{
    int bgrTable[3];
    if (rawFrame.yuv) {
        bgrTable[0] = 0;
        bgrTable[1] = 1;
        bgrTable[2] = 2;
    } else {
        bgrTable[0] = 2;
        bgrTable[1] = 0;
        bgrTable[2] = 1;
    }
    for (int i = 0; i < 3; ++i) {
        int jpegIndex = 0;
        unsigned char *dataPtr = (unsigned char*)&rawFrame.videoData[bgrTable[i]].front();

        int h = _heightUsed;
        int w = _widthUsed;
        if (i > 0 && _subsampledChroma) {
            h /= 2;
            w /= 2;
        }

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (_targetBitDepth == BR_8bit) {

                    image->comps[i].data[jpegIndex] = dataPtr[y * w + x];

                } else if (_targetBitDepth == BR_10bit || _targetBitDepth == BR_12bit) {
                    int rgbIndex = x * 2;
                    unsigned char c1 = dataPtr[y * 2 * w + (rgbIndex + 0)];
                    unsigned char c2 = dataPtr[y * 2 * w + (rgbIndex + 1)];
                    unsigned short color = (unsigned short)(c2 << 8) + c1;
                    image->comps[i].data[jpegIndex] = (unsigned short)color;
                }
                jpegIndex++;
            }
        }
    }
}

bool J2KEncoder::EncodeImage(opj_image_t *image, J2kFrame &encodedFrame)
{

    opj_codec_t* codec = opj_create_compress((OPJ_CODEC_FORMAT) 0);
    if (codec == nullptr) {
        std::cerr << "[J2K] error creating j2k codec" << std::endl;
        return false;
    }

    //opj_set_info_handler(codec, [](const char* s, void *) { std::cout << "[J2K] " << s; }, nullptr);
    opj_set_warning_handler(codec, [](const char* s, void *) { std::cerr << "[J2K] " << s; }, nullptr);
    opj_set_error_handler(codec, [](const char* s, void *) { std::cerr << "[J2K] " << s; }, nullptr);

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
        std::cerr << "[J2K] ERROR opj_start_compress" << std::endl;
    } else {
        success = success && opj_encode(codec, stream);
        if (success == false) {
            std::cerr << "[J2K] ERROR opj_encode" << std::endl;
        }
    }
    success = success && opj_end_compress(codec, stream);
    if (success == false)  {
        std::cerr << "[J2K] failed to encode image: opj_end_compress" << std::endl;
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
    _encodingParameters.tcp_mct = (int)_mct;        // I really wonder what this does???
    if (!_subsampledChroma) {
        _encodingParameters.subsampling_dx = 1;
        _encodingParameters.subsampling_dy = 1;
    } else {
        _encodingParameters.subsampling_dx = 2;
        _encodingParameters.subsampling_dy = 1;
    }


    std::cout << "[J2K] MCT enabled: " << _mct << std::endl;
    std::cout << "Subsampling x/y : " << _encodingParameters.subsampling_dx << "/" << _encodingParameters.subsampling_dy << std::endl;

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

    switch (_profile) {
        case PROFILE::BCP_ST_1:
            // SetRate(
        case PROFILE::BCP_ST_2:
        case PROFILE::BCP_ST_3:
        case PROFILE::BCP_ST_4:
        case PROFILE::BCP_ST_5:
            if (_useTiles) {
                throw std::runtime_error("[J2K] single tile broadcast profile cant be used with tiles");
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
                std::cout << "[J2K] Use tiled encoding" << std::endl;
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
        std::cout << "[J2K] tcp_rates[0]: " << _encodingParameters.tcp_rates[0] << std::endl;
    } else {
        std::cout << "[J2K] lossless compression" << std::endl;
    }
}
