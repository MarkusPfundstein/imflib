#ifndef IMFPACKAGEVIEW_H
#define IMFPACKAGEVIEW_H

#include <QMainWindow>
#include <QMenu>
#include "qtmodels/imfpackagemodel.h"

class IMFPackageView : public QMainWindow
{
    Q_OBJECT

    public:
        IMFPackageView();
        virtual ~IMFPackageView();

        // disables and enables all menus according to state of program
        void UpdateMenu();

    protected:
    private slots:
        // File Menu
        void NewFile();
        void OpenFile();
        void SaveFile();

        // IMF Menu
        void AddTrackFile();

    private:
        void CreateActions();
        void CreateMenus();

        QMenu *_fileMenu;
        QMenu *_imfMenu;

        QAction *_newFileAction;
        QAction *_openFileAction;
        QAction *_saveFileAction;

        QAction *_addTrackFileAction;

        IMFPackageModel _packageModel;
};

#endif // IMFPACKAGEVIEW_H
