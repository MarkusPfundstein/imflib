#ifndef IMFOUTPUTPROFILE_H
#define IMFOUTPUTPROFILE_H

#include "imfpackageitem.h"

class IMFOutputProfile : public IMFPackageItem
{
    public:
        IMFOutputProfile(const std::string& filename);
        virtual ~IMFOutputProfile();

    protected:
    private:
};

#endif // IMFOUTPUTPROFILE_H
