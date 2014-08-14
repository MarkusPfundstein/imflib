#ifndef IMFTRACK_H
#define IMFTRACK_H

#include "imfpackageitem.h"

class IMFTrack : public IMFPackageItem
{
    public:
        IMFTrack(const std::string& filename);
        virtual ~IMFTrack();

    protected:


};

#endif // IMFTRACK_H
