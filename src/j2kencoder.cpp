#include "j2kencoder.h"
#include "rawvideoframe.h"


#include <stdexcept>
#include <iostream>
#include <cstring> // memset

#include <functional>

J2KEncoder::J2KEncoder()
{
    //ctor
}

J2KEncoder::~J2KEncoder()
{
    //dtor
}

bool J2KEncoder::EncodeRawFrame(RawVideoFrame &rawFrame, J2kFrame &encodedFrame)
{
    opj_cparameters_t encodingParameters;
    opj_set_default_encoder_parameters(&encodingParameters);
    encodingParameters.cp_tx0 = 0;
    encodingParameters.cp_ty0 = 0;
    encodingParameters.subsampling_dx = 1;
    encodingParameters.subsampling_dy = 1;
    encodingParameters.tcp_mct = 1;         // 0 = store as rgb, 1 = store as yuv ??? I THINK!!! :-)
    std::cout << (encodingParameters.tcp_mct == 0 ? "STORE AS RGB" : "STORE AS YUV") << std::endl;

    // store here string for code stream commment. must be on heap. gets FREE'd below (not delete'd).
    // if you dont do it it gets allocated anyway . lol
    //encodingParameters.cp_comment = (char*)malloc(blabla)

    // without this we get segfault. no idea why -> TO-DO: find out :-)
    // but what stands there means that we store lossless. 1 layer, comp. rate of that layer = 0
    if (encodingParameters.tcp_numlayers == 0) {
        encodingParameters.tcp_rates[0] = 0;	// MOD antonin : losslessbug
        encodingParameters.tcp_numlayers++;
        encodingParameters.cp_disto_alloc = 1;
    }

    // IMF profiles - TO-DO: they dont do anything yet. thus are not implemented into jpeg2000. lets fix this later
    //encodingParameters.rsiz = OPJ_PROFILE_IMF_2K_R;
    //encodingParameters.rsiz = OPJ_PROFILE_IMF_4K_R;
    //encodingParameters.rsiz = OPJ_PROFILE_IMF_8K_R;

    // We get RGB24 image data. Thus 3 components, 8 bit per component
    int numberComponents = 3;
    int rawBitDepth = 8;

    // doesnt seem to affect the output file
    OPJ_COLOR_SPACE colorSpace = OPJ_COLOR_SPACE::OPJ_CLRSPC_SRGB;
    //OPJ_COLOR_SPACE colorSpace = OPJ_COLOR_SPACE::OPJ_CLRSPC_SYCC;

    // no idea
    // int rawSigned = true;

    // ??? seriously ???
    int bigEndian = false;

    int widthUsed = rawFrame.width;
    int heightUsed = rawFrame.height;

    opj_image_cmptparm_t *componentParameter = (opj_image_cmptparm_t*) malloc((size_t)numberComponents * sizeof(opj_image_cmptparm_t));
    memset(&componentParameter[0], 0, (size_t)numberComponents * sizeof(opj_image_cmptparm_t));

    for (int i = 0; i < numberComponents; ++i) {
        componentParameter[i].prec = (uint32_t) rawBitDepth;
        componentParameter[i].bpp = (uint32_t) rawBitDepth;
        componentParameter[i].sgnd = (uint32_t) bigEndian;
        componentParameter[i].dx = (uint32_t) encodingParameters.subsampling_dx * 1;
        componentParameter[i].dy = (uint32_t) encodingParameters.subsampling_dy * 1;
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
            int rgbIndex = x * 3;
            image->comps[0].data[jpegIndex] = dataPtr[y * 3 * widthUsed + (rgbIndex + 0)];	// R
            image->comps[1].data[jpegIndex] = dataPtr[y * 3 * widthUsed + (rgbIndex + 1)];	// G
            image->comps[2].data[jpegIndex] = dataPtr[y * 3 * widthUsed + (rgbIndex + 2)];	// B
            jpegIndex++;
        }
    }

    bool success = EncodeImage(image, encodedFrame, encodingParameters);

    opj_image_destroy(image);

    // this gets allocated in openjpeg lib. its seriously ridicolous :D nothing mentioned
    // in the api or anywhere. nevertheless, we have to free it here
    if (encodingParameters.cp_comment != nullptr) {
        free(encodingParameters.cp_comment);
    }

    return success;
}

bool J2KEncoder::EncodeImage(opj_image_t *image, J2kFrame &encodedFrame, opj_cparameters_t &parameters)
{

    opj_codec_t* codec = opj_create_compress((OPJ_CODEC_FORMAT) 0);
    if (codec == nullptr) {
        std::cout << "error creating j2k codec" << std::endl;
        return false;
    }

    opj_set_info_handler(codec, [](const char* s, void *) { std::cout << "[I]" << s << std::endl; }, nullptr);
    opj_set_warning_handler(codec, [](const char* s, void *) { std::cout << "[I]" << s << std::endl; }, nullptr);
    opj_set_error_handler(codec, [](const char* s, void *) { std::cout << "[I]" << s << std::endl; }, nullptr);


    opj_setup_encoder(codec, &parameters, image);


    // TO-DO: Move from file stream to memory stream
    //opj_stream_t* stream = opj_stream_create_default_file_stream(DEBUG_OUT_FILE.c_str(), false);
    opj_stream_t* stream = opj_stream_default_create(false);
    if (stream == nullptr) {
        opj_destroy_codec(codec);
        return false;
    }

    opj_stream_set_user_data(stream, &encodedFrame, nullptr);
    opj_stream_set_write_function(stream, &J2KEncoder::WriteJ2kFrame);


    // TO-DO : ADD TILE SUPPORT

    std::cout << "start compress" << std::endl;
    bool success = opj_start_compress(codec, image, stream);
    if (success == false)  {
        std::cout << "ERROR opj_start_compress" << std::endl;
    } else {
        std::cout << "start encode" << std::endl;
        success = success && opj_encode(codec, stream);
        if (success == false) {
            std::cout << "ERROR opj_encode" << std::endl;
        }
    }
    success = success && opj_end_compress(codec, stream);
    if (success == false)  {
        std::cout << "failed to encode image: opj_end_compress" << std::endl;
    }

    opj_stream_destroy(stream);
    opj_destroy_codec(codec);

    return true;
}

OPJ_SIZE_T J2KEncoder::WriteJ2kFrame(void *data, OPJ_SIZE_T bufferSize, void *userData)
{
    J2kFrame *frame = static_cast<J2kFrame*>(userData);
    frame->data.reserve(bufferSize);
    std::copy((uint8_t*)data, (uint8_t*)data + bufferSize, std::back_inserter(frame->data));

    return bufferSize;
}
