#ifndef IMFPACKAGELIST_H
#define IMFPACKAGELIST_H

#include "imfpackageitem.h"
#include "xmlheader.h"
#include <vector>
#include <memory>

class IMFPackageItem;

class IMFPackageList: public IMFPackageItem
{
    public:
        IMFPackageList(const std::string &uuid, const std::string& filename);
        virtual ~IMFPackageList();

	void Write(const std::vector<std::shared_ptr<IMFPackageItem>> &assets);

    protected:
    private:

	XMLHeaderPackageList _header;
};

#endif // IMFOUTPUTPROFILE_H
