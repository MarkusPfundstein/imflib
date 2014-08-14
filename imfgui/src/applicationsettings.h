#ifndef APPLICATIONSETTINGS_H
#define APPLICATIONSETTINGS_H

#include <QString>

struct ApplicationSettings
{
    public:
        ApplicationSettings();
        ~ApplicationSettings();
        ApplicationSettings(const ApplicationSettings& other) = delete;
        ApplicationSettings& operator=(const ApplicationSettings& other) = delete;

        // returns string to settings file for user
        QString GetSettingsFileLocation() const;

        // loads application settings for user.
        void LoadSettings();
        void SaveSettings();

        QString GetLastOpenedTrackDir() const;
        void SetLastOpenedTrackDir(QString dir);

    private:
        // last directory where user opened a track file
        QString _lastOpenedTrackDir;
};

#endif // APPLICATIONSETTINGS_H
