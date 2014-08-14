#ifndef APPLICATION_H
#define APPLICATION_H

#include "applicationsettings.h"

#include <QApplication>
#include <memory>
#include <QString>

// forward references
class IMFPackage;

class Application : public QApplication
{
    public:
        Application(int &argc, char **argv);
        virtual ~Application();

        void SetWorkingPackage(IMFPackage *package);

        ApplicationSettings* Settings();

    protected:
    private:
        // settings stored for the user
        ApplicationSettings _settings;

        // current package we are working with
        std::unique_ptr<IMFPackage> _workingPackage;
};

#endif // APPLICATION_H
