#include <QApplication>
#include "views/imfpackageview.h"
#include "application.h"

int main(int argv, char **args)
{
    Application app(argv, args);

    IMFPackageView packageView;
    packageView.show();
    return app.exec();
 }
