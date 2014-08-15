#ifndef MXFREADER_H
#define MXFREADER_H

#include <string>
#include <memory>
#include <stdexcept>

// forward declarations
class IMFVideoTrack;
class IMFAudioTrack;

class MXFReaderException : public std::runtime_error
{
    public:
        MXFReaderException(std::string const& error)
        : std::runtime_error(error) {};
};

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

        // parses headers in track
        void ParseMetadata(const std::shared_ptr<IMFVideoTrack> &track);
        void ParseMetadata(const std::shared_ptr<IMFAudioTrack> &track);
    protected:
    private:
        std::string _filename;
};

#endif // MXFREADER_H
