#ifndef IMFCOMPOSITIONPLAYLIST_H
#define IMFCOMPOSITIONPLAYLIST_H

#include "imfpackageitem.h"
#include "imfvideotrack.h"
#include "imfaudiotrack.h"

#include <vector>
#include <memory>

#include <boost/property_tree/ptree.hpp>

class CPLResource;

class IMFCompositionPlaylistException : public std::runtime_error
{
    public:
        IMFCompositionPlaylistException(std::string const& error)
        : std::runtime_error(error) {};
};

class IMFCompositionPlaylist : public IMFPackageItem
{
    public:
        IMFCompositionPlaylist(const std::string &uuid, const std::string& filename);
        virtual ~IMFCompositionPlaylist();

        // writes composition playlist to disk
        void Write() const;

        // [STATIC] loads composition playlist from disk. Throws all boost::property_tree and xml_parser exceptions...
        static std::shared_ptr<IMFCompositionPlaylist> Load(const std::string &filename,
                                                             boost::property_tree::ptree& pt,
                                                             const std::vector<std::shared_ptr<IMFTrack>> &tracks);

        RationalNumber GetEditRate() const
        { return _editRate; }

        void SetEditRate(RationalNumber n)
        { _editRate = n; }

    protected:
    private:
        std::shared_ptr<CPLResource> LoadCPLResource(const boost::property_tree::ptree &pt,
                                                     const std::string &cplEditRate,
                                                     const std::vector<std::shared_ptr<IMFTrack>> &tracks);

        // global edit rate of playlist
        RationalNumber _editRate;
};

#endif // IMFCOMPOSITIONPLAYLIST_H
