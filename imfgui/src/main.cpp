#include <QApplication>
#include "views/imfpackageview.hpp"

int main(int argv, char **args)
{
    QApplication app(argv, args);
    IMFPackageView packageView;
    packageView.show();
    return app.exec();
 }
