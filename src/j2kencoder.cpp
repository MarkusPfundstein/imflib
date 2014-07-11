#include "j2kencoder.h"
#include "rawvideoframe.h"

extern "C" {
#include <openjpeg.h>
}

#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstring> // memset

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
    encodingParameters.subsampling_dx = 1;
    encodingParameters.subsampling_dy = 1;
    encodingParameters.tcp_mct = 0;         // ?????
    encodingParameters.image_offset_x0 = 0;
    encodingParameters.image_offset_y0 = 0;

    // We get RGB24 image data. Thus 3 components, 8 bit per component
    int numberComponents = 3;
    int rawBitDepth = 8;
    OPJ_COLOR_SPACE colorSpace = OPJ_COLOR_SPACE::CLRSPC_SRGB;

    // no idea
    int rawSigned = true;

    // ??? seriously ???
    int bigEndian = false;

    opj_image_cmptparm_t *componentParameter = (opj_image_cmptparm_t*) malloc((size_t)numberComponents * sizeof(opj_image_cmptparm_t));
    memset(&componentParameter[0], 0, (size_t)numberComponents * sizeof(opj_image_cmptparm_t));

    for (int i = 0; i < numberComponents; ++i) {
        componentParameter[i].prec = (uint32_t) rawBitDepth;
        componentParameter[i].bpp = (uint32_t) rawBitDepth;
        componentParameter[i].sgnd = (uint32_t) bigEndian;
        componentParameter[i].dx = (uint32_t) encodingParameters.subsampling_dx * 1;
        componentParameter[i].dy = (uint32_t) encodingParameters.subsampling_dy * 1;
        componentParameter[i].w = (uint32_t) rawFrame.width;
        componentParameter[i].h = (uint32_t) rawFrame.height;
    }

    // create image. NOTE: THIS IMAGE MUST BE FREE'D!!!
    opj_image_t *image = opj_image_create((uint32_t) numberComponents, &componentParameter[0], colorSpace);
    free(componentParameter);
    if(!image) {
        throw std::runtime_error("error creating jpeg2k image");
    }

    image->x0 = (uint32_t) encodingParameters.image_offset_x0;
    image->y0 = (uint32_t) encodingParameters.image_offset_y0;
    image->x1 = (uint32_t) encodingParameters.image_offset_x0 + (uint32_t) (rawFrame.width - 1) * (uint32_t) encodingParameters.subsampling_dx + 1;
    image->y1 = (uint32_t) encodingParameters.image_offset_y0 + (uint32_t) (rawFrame.height - 1) * (uint32_t) encodingParameters.subsampling_dy + 1;

    unsigned char* dataPtr = rawFrame.videoData[0];

    if (rawBitDepth == 8) {
        unsigned char value = 0;
        for (int i = 0; i < numberComponents; ++i) {
            int loop = (rawFrame.width * rawFrame.height);
            for (int j = 0; j < loop; ++j) {
                unsigned char t = dataPtr[i * loop + j];
                image->comps[i].data[j] = rawSigned ? (char)value : value;
            }
        }
    } else {
        std::cout << "[J2kEncoder] Unsupported bitrate" << std::endl;
        opj_image_destroy(image);
        return false;
    }

    opj_image_destroy(image);


    return true;
}
