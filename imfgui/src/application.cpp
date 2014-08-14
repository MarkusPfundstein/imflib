#include "application.h"

#include "model/imfpackage.h"

#include <QSettings>
#include <QDir>

Application::Application(int &argc, char **argv)
    :
    QApplication(argc, argv),
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
