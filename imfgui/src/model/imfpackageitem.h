#ifndef IMFPACKAGEITEM_H
#define IMFPACKAGEITEM_H

#include "genericitem.h"

class IMFPackageItem : public GenericItem
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

        const std::string& GetPath() const
        { return _filename; }

        void SetPath(const std::string& p)
        { _filename = p; }

        TYPE GetType() const
        { return _type; }

        virtual std::string TypeString() const;

        // returns _filename without path
        std::string GetFileName() const;

        // returns file size
        int GetFileSize() const;

    protected:
        std::string _filename;

        TYPE _type;

};

#endif // IMFPACKAGEITEM_H
