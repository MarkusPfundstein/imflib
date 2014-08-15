#ifndef IMFPACKAGE_H
#define IMFPACKAGE_H

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>


// forward references
class IMFVideoTrack;
class IMFAudioTrack;
class IMFOutputProfile;
class IMFCompositionPlaylist;

class IMFPackageException : public std::runtime_error
{
    public:
        IMFPackageException(std::string const& error)
        : std::runtime_error(error) {};
};

class IMFPackage
{
    public:
        IMFPackage();
        virtual ~IMFPackage();

        IMFPackage(const IMFPackage& other) = delete;
        IMFPackage& operator=(const IMFPackage& other) = delete;

        // adds a new video track to the package
        void AddVideoTrack(const std::shared_ptr<IMFVideoTrack> &track);

        // adds a new audio track to the package
        void AddAudioTrack(const std::shared_ptr<IMFAudioTrack> &track);

        // checks if a track file is already contained in the package
        bool HasTrackFile(const std::string &file) const;

        // checks if a video track file is already contained in the package
        bool HasVideoTrackFile(const std::string &file) const;

        // checks if a audio track file is already contained in the package
        bool HasAudioTrackFile(const std::string &file) const;

        // writes whole package to disk
        void Write() const;

        std::string GetName() const
        { return _name; }

        void SetName(const std::string &n)
        { _name = n; }

        bool IsSaved() const
        { return _saved; }

        void SetSaved(bool s)
        { _saved = s; }

        std::string GetLocation() const
        { return _location; }

        void SetLocation(const std::string &s)
        { _location = s; }

        std::string GetUUID() const
        { return _uuid; }

        void SetUUID(const std::string& uuid)
        { _uuid = uuid; }

    protected:
    private:

        // writes asset map
        void WriteAssetMap(const std::string &filename) const;

        // name of imf package
        std::string _name;

        // changes saved?
        bool _saved;

        // path to it
        std::string _location;

        // UUID
        std::string _uuid;

        // array of video tracks
        std::vector<std::shared_ptr<IMFVideoTrack>> _videoTracks;

        // array of audio tracks
        std::vector<std::shared_ptr<IMFAudioTrack>> _audioTracks;

        // array of composition playlists
        std::vector<std::shared_ptr<IMFCompositionPlaylist>> _compositionPlaylists;

        // array of output profiles
        std::vector<std::shared_ptr<IMFOutputProfile>> _outputProfiles;
};

#endif // IMFPACKAGE_H
