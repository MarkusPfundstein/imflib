#include "applicationsettings.h"

#include <QDir>
#include <QSettings>

ApplicationSettings::ApplicationSettings()
    :
    _lastOpenedTrackDir(QDir::homePath()),
    _lastSaveDir(QDir::homePath()),
    _lastOpenDir(QDir::homePath())
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
    _lastSaveDir = settings.value("last_save_dir", _lastSaveDir).toString();
    _lastOpenDir = settings.value("last_open_dir", _lastOpenDir).toString();
}

void ApplicationSettings::SaveSettings()
{
    QString settingsFile = GetSettingsFileLocation();

    QSettings settings(settingsFile, QSettings::NativeFormat);

    settings.setValue("last_opened_track_dir", _lastOpenedTrackDir);
    settings.setValue("last_save_dir", _lastSaveDir);
    settings.setValue("last_open_dir", _lastOpenDir);
}
