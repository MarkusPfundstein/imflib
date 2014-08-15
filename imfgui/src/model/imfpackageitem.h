#ifndef IMFPACKAGEITEM_H
#define IMFPACKAGEITEM_H

#include <string>

class IMFPackageItem
{
    public:
        enum TYPE {
            VIDEO = 1,
            AUDIO = 2,
            CPL = 3,
            OPL = 4
        };

        explicit IMFPackageItem(const std::string &filename, TYPE type);
        virtual ~IMFPackageItem();

        const std::string& GetFileName() const
        { return _filename; }

        TYPE GetType() const
        { return _type; }

        std::string TypeString() const;

    protected:
        std::string _filename;

        TYPE _type;
};

#endif // IMFPACKAGEITEM_H
