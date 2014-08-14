#ifndef MXFREADER_H
#define MXFREADER_H

#include <string>

class MXFReader
{
    public:
        // type of essence in mxf file
        enum ESSENCE_TYPE {
            INVALID = -1,
            VIDEO = 1,
            AUDIO = 2
        };

        MXFReader(const std::string& filename);
        virtual ~MXFReader();
        MXFReader(const MXFReader& other) = delete;
        MXFReader& operator=(const MXFReader& other) = delete;

        // parses mxf file _filename and returns essence type
        ESSENCE_TYPE GetEssenceType() const;
    protected:
    private:

        std::string _filename;
};

#endif // MXFREADER_H
