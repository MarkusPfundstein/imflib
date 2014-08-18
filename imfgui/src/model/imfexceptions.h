#ifndef IMFEXCEPTIONS_H_INCLUDED
#define IMFEXCEPTIONS_H_INCLUDED

#include <stdexcept>

class IMFCompositionPlaylistException : public std::runtime_error
{
    public:
        IMFCompositionPlaylistException(std::string const& error)
        : std::runtime_error(error) {};
};

class IMFPackageException : public std::runtime_error
{
    public:
        IMFPackageException(std::string const& error)
        : std::runtime_error(error) {};
};

class IMFInvalidReferenceException : public std::runtime_error
{
    public:
        IMFInvalidReferenceException(std::string const& error)
        : std::runtime_error(error) {};
};

#endif // IMFEXCEPTIONS_H_INCLUDED
