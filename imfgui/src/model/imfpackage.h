#ifndef IMFPACKAGE_H
#define IMFPACKAGE_H

#include <memory>
#include <vector>

// forward references
class IMFVideoTrack;
class IMFAudioTrack;
class IMFOutputProfile;
class IMFCompositionPlaylist;

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
    protected:
    private:

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
