#ifndef MXFWRITER_H
#define MXFWRITER_H

#include <string>
#include <map>
#include <boost/any.hpp>
#include <list>

#include "common.h"

class MXFWriter
{
    public:
        typedef struct {
            RationalNumber editRate;
            RationalNumber aspectRatio;
            uint32_t containerDuration;
            bool yuvEssence;
            int subsamplingDx;
            bool encryptHeader;
            int bits;
            int broadcastProfile;
            bool fullRange;
        }  MXFOptionsVideo;

        typedef struct {
            RationalNumber editRate;
        } MXFOptionsAudio;

        MXFWriter();
        virtual ~MXFWriter();

        void MuxAudioFile(const std::string& file, const std::string &finalFile, const MXFOptionsAudio &options);
        void MuxVideoFiles(const std::list<std::string> &files, const std::string &finalFile, const MXFOptionsVideo &options);
};

#endif // MXFWRITER_H
