#include "applicationsettings.h"

#include <QDir>
#include <QSettings>

ApplicationSettings::ApplicationSettings()
    :
    _lastOpenedTrackDir(QDir::homePath())
{
    //ctor
}

ApplicationSettings::~ApplicationSettings()
{
    //dtor
}

QString ApplicationSettings::GetSettingsFileLocation() const
{
    QString homeLocation = QDir::homePath();
    return homeLocation + "/.imfguisettings";
}

void ApplicationSettings::LoadSettings()
{
    QString settingsFile = GetSettingsFileLocation();

    QSettings settings(settingsFile, QSettings::NativeFormat);

    _lastOpenedTrackDir = settings.value("last_opened_track_dir", _lastOpenedTrackDir).toString();
}

void ApplicationSettings::SaveSettings()
{
    QString settingsFile = GetSettingsFileLocation();

    QSettings settings(settingsFile, QSettings::NativeFormat);

    settings.setValue("last_opened_track_dir", _lastOpenedTrackDir);
}

QString ApplicationSettings::GetLastOpenedTrackDir() const
{
    return _lastOpenedTrackDir;
}

void ApplicationSettings::SetLastOpenedTrackDir(QString dir)
{
    _lastOpenedTrackDir = dir;
}
