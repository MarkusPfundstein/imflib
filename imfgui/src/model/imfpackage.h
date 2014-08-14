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

        void AddVideoTrack(const std::shared_ptr<IMFVideoTrack> &track);
        void AddAudioTrack(const std::shared_ptr<IMFAudioTrack> &track);
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
