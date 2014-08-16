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

        const std::string& GetPath() const
        { return _filename; }

        void SetPath(const std::string& p)
        { _filename = p; }

        TYPE GetType() const
        { return _type; }

        const std::string& GetUUID() const
        { return _uuid; }

        void SetUUID(const std::string &u)
        { _uuid = u; }

        virtual std::string TypeString() const;

        // returns _filename without path
        std::string GetFileName() const;

        // returns file size
        int GetFileSize() const;

    protected:
        std::string _uuid;
        std::string _filename;

        TYPE _type;

};

#endif // IMFPACKAGEITEM_H
