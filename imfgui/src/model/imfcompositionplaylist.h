#ifndef IMFCOMPOSITIONPLAYLIST_H
#define IMFCOMPOSITIONPLAYLIST_H

#include "imfpackageitem.h"
#include "imfvideotrack.h"
#include "imfaudiotrack.h"

#include <vector>
#include <memory>

#include <boost/property_tree/ptree.hpp>

class IMFCompositionPlaylist : public IMFPackageItem
{
    // stores a IMFVideoTrack and/or AudioTrack with associated metadata for a sequence
    struct TrackContainer
    {
        TrackContainer() : track(), start(0), entryPoint(0), length(0), repeatCount(0) {}

        // pointer to the track to store
        std::shared_ptr<IMFTrack> track;

        /* track metadata */

        // start position in playlist
        int start;

        // entry point
        int entryPoint;

        // length position
        int length;

        // repeat count
        int repeatCount;
    };

    // stores a sequence of Tracks
    class Segment
    {
        public:
            Segment(const std::string &uuid) : _uuid(uuid), _videoTracks(), _audioTracks() {}

        private:

            std::string _uuid;

            std::vector<std::shared_ptr<TrackContainer>> _videoTracks;
            std::vector<std::shared_ptr<TrackContainer>> _audioTracks;
    };

    public:
        IMFCompositionPlaylist(const std::string& filename);
        virtual ~IMFCompositionPlaylist();

        // writes composition playlist to disk
        void Write() const;

        // [STATIC] loads composition playlist from disk. Throws all boost::property_tree and xml_parser exceptions...
        static std::shared_ptr<IMFCompositionPlaylist> Load(const std::string &filename, boost::property_tree::ptree& pt);

        RationalNumber GetEditRate() const
        { return _editRate; }

        void SetEditRate(RationalNumber n)
        { _editRate = n; }

    protected:
    private:
        // global edit rate of playlist
        RationalNumber _editRate;

        // video and tracks in container.
        std::vector<std::shared_ptr<Segment>> _segments;
};

#endif // IMFCOMPOSITIONPLAYLIST_H
