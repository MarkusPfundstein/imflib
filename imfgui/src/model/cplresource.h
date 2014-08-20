#ifndef CPLRESOURCE_H
#define CPLRESOURCE_H

#include "genericitem.h"
#include "imftrack.h"
#include "imfexceptions.h"

#include <memory>
#include <string>

#include <vector>
#include <boost/property_tree/ptree.hpp>

// SMPTE ST 2067-3 defines this as a base class where
// another class TrackFileResource derives from.
// I ignore that from now and assume each resource is a CPLResource.
// In the standard, there is no mentioning of other resource types
// than TrackFileResourceType
class CPLResource : public GenericItem
{
    public:
        CPLResource(const std::string &uuid, const std::shared_ptr<IMFTrack> &track);
        virtual ~CPLResource();

        // loads a CPLResource from a property tree reference. Attentation: throws all xml exceptions
        static std::shared_ptr<CPLResource> Load(const boost::property_tree::ptree &pt,
                                                  const std::string &cplEditRate,
                                                  const std::vector<std::shared_ptr<IMFTrack>> &tracks);

        static std::shared_ptr<CPLResource> StandardResource(const std::shared_ptr<IMFTrack> &track,
                                                              RationalNumber compositionPlaylistEditRate);

        // returns real playing time of referenced resource
        int GetIntrinsicDuration() const
        { return _track->GetDuration(); }

        // gets source duration but normalized to video frames.
        // if resource is audio it returns
        // intrinsic duration / fps * playlistEditRate
        int GetNormalizedSourceDuration() const;

        // returns edit rate of (referenced resource??).
        // this a bit unclear yet if this always must be the edit rate of the referenced resource
        // or if this would overwrite the edit rate of it. so that a resource in the cpl can
        // have a different edit rate then the resource it refers too -> with the player doing
        // the conversion????
        RationalNumber GetEditRate() const
        { return _track->GetEditRate(); }

        // returns UUID of referenced resource
        const std::string& GetTrackFileId() const
        { return _track->GetUUID(); }

        int GetEntryPoint() const
        { return _entryPoint; }

        void SetEntryPoint(int p)
        { _entryPoint = p; }

        int GetSourceDuration() const
        { return _sourceDuration; }

        void SetSourceDuration(int d)
        { _sourceDuration = d; }

        const std::shared_ptr<IMFTrack> &GetTrack() const
        { return _track; }

        const std::string &GetSourceEncoding() const
        { return _sourceEncoding; }

        void SetSourceEncoding(const std::string &e)
        { _sourceEncoding = e; }

        const std::string &GetKeyId() const
        { return _keyId; }

        void SetKeyId(const std::string& k)
        { _keyId = k; }

        const std::string &GetHash() const
        { return _hash; }

        void SetHash(const std::string &h)
        { _hash = h; }

        int GetRepeatCount() const
        { return _repeatCount; }

        void SetRepeatCount(int r)
        { _repeatCount = r; }

        RationalNumber GetPlaylistEditRate() const
        { return _playlistEditRate; }

        void SetPlaylistEditRate(RationalNumber r)
        { _playlistEditRate = r; }

    protected:
    private:
        /*
        SMPTE ST 2067-3 defines the following properties
        - resourceId
        - intrinsic duration (we get that from IMFTrack -> duration)
        - entry point
        - source duration
        - edit rate (we get that from IMFTrack -> editRate)
        - source encoding
        - track file id
        - key id
        - hash
        */

        std::shared_ptr<IMFTrack> _track;

        int _repeatCount;
        int _entryPoint;
        int _sourceDuration;
        std::string _sourceEncoding;
        std::string _keyId;
        std::string _hash;

        RationalNumber _playlistEditRate;


};

#endif // CPLRESOURCE_H
