#include <QApplication>
#include "imfpackageview.hpp"

int main(int argv, char **args)
{
    QApplication app(argv, args);
    IMFPackageView packageView;
    packageView.show();
    return app.exec();
 }
