#include "application.h"

#include "model/imfpackage.h"

#include <QSettings>
#include <QDir>

Application::Application(int &_argc, char **_argv)
    :
    QApplication(_argc, _argv),
    _settings(),
    _workingPackage(nullptr)
{
    //ctor
    _settings.LoadSettings();
}

Application::~Application()
{
    //dtor
    _settings.SaveSettings();
}

void Application::SetWorkingPackage(IMFPackage* package)
{
    _workingPackage.reset(package);
}

ApplicationSettings* Application::Settings()
{
    return &_settings;
}

IMFPackage* Application::GetWorkingPackage() const
{
    return _workingPackage.get();
}
