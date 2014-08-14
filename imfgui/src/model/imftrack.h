#ifndef IMFTRACK_H
#define IMFTRACK_H

#include <string>

class IMFTrack
{
    public:
        IMFTrack(const std::string& filename);
        virtual ~IMFTrack();

        const std::string& GetFileName() const
        { return _filename; }

    protected:

        std::string _filename;
};

#endif // IMFTRACK_H
