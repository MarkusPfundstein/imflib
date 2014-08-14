#ifndef IMFPACKAGEITEM_H
#define IMFPACKAGEITEM_H

#include <string>

class IMFPackageItem
{
    public:
        explicit IMFPackageItem(const std::string &filename);
        virtual ~IMFPackageItem();

        const std::string& GetFileName() const
        { return _filename; }

    protected:
        std::string _filename;
};

#endif // IMFPACKAGEITEM_H
