#ifndef IMFAUDIOTRACK_H
#define IMFAUDIOTRACK_H

#include "imftrack.h"
#include "imfessencedescriptor.h"


class IMFAudioTrack : public IMFTrack
{
    public:
        IMFAudioTrack(const std::string &uuid, const std::string& filename, std::shared_ptr<IMFEssenceDescriptor> essenceDescriptor);
        virtual ~IMFAudioTrack();
    protected:
    private:
};

#endif // IMFAUDIOTRACK_H
