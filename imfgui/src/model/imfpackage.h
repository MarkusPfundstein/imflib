#ifndef IMFPACKAGE_H
#define IMFPACKAGE_H

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>

#include "imfexceptions.h"
#include "xmlheader.h"

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

        // parses and adds a track
        void ParseAndAddTrack(const std::string &name);

    private:
        // keep it private for the time being. dont think we need it in public API
        // parses and adds a xml (CPL/OPL/PKL)
        void ParseAndAddXML(const std::string &name);

    public:
        // adds a new video track to the package
        void AddVideoTrack(const std::shared_ptr<IMFVideoTrack> &track);

        // adds a new audio track to the package
        void AddAudioTrack(const std::shared_ptr<IMFAudioTrack> &track);

        // adds a new composition playlist to the package
        void AddCompositionPlaylist(const std::shared_ptr<IMFCompositionPlaylist> &track);

        // checks if a track file is already contained in the package
        bool HasTrackFile(const std::string &file) const;

        // checks if a video track file is already contained in the package
        bool HasVideoTrackFile(const std::string &file) const;

        // checks if a audio track file is already contained in the package
        bool HasAudioTrackFile(const std::string &file) const;

        // writes whole package to disk
        void Write() const;

        // copies all files into package directory
        void CopyTrackFiles() const;

        // loads imfpackage from disk
        void Load(const std::string& location);

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

        std::string GetFullPath() const
        { return _location + "/" + _name; }

        std::string GetUUID() const
        { return _uuid; }

        void SetUUID(const std::string& uuid)
        { _uuid = uuid; }

        const std::vector<std::shared_ptr<IMFVideoTrack>>& GetVideoTracks() const { return _videoTracks; }
        const std::vector<std::shared_ptr<IMFAudioTrack>>& GetAudioTracks() const { return _audioTracks; }
        const std::vector<std::shared_ptr<IMFCompositionPlaylist>>& GetCompositionPlaylists() const { return _compositionPlaylists; }
        const std::vector<std::shared_ptr<IMFOutputProfile>>& GetOutputProfiles() const { return _outputProfiles; }

    protected:
    private:

        // writes asset map
        void WriteAssetMap(const std::string &filename) const;

        // reads asset map
        void ReadAssetMap(const std::string &filename);


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

        XMLHeaderAssetMap _headerAssetMap;
};

#endif // IMFPACKAGE_H
