#ifndef IMFCOMPOSITIONPLAYLIST_H
#define IMFCOMPOSITIONPLAYLIST_H

#include "imfpackageitem.h"
#include "imfvideotrack.h"
#include "imfaudiotrack.h"
#include "imfexceptions.h"

#include <vector>
#include <memory>

#include <boost/property_tree/ptree.hpp>

class CPLSegment;
class CPLVirtualTrack;
class CPLResource;

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

        // pushes a segment onto list
        void AddSegment(const std::shared_ptr<CPLSegment> &segment);

        // pushes a virtual track onto list
        void AddVirtualTrack(const std::shared_ptr<CPLVirtualTrack> &vt);

        std::shared_ptr<CPLVirtualTrack> FindVirtualTrackById(const std::string &id) const;

        // checks if a virtual track with a certain id exists already
        bool VirtualTrackExists(const std::string &id) const;

    protected:
    private:
        // global edit rate of playlist
        RationalNumber _editRate;

        std::vector<std::shared_ptr<CPLSegment>> _segments;
        std::vector<std::shared_ptr<CPLVirtualTrack>> _virtualTracks;


};

#endif // IMFCOMPOSITIONPLAYLIST_H
