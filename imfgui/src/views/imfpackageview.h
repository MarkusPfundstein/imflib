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
        void NewCompositionPlaylist();

    private:

        // creates all actions. gets called on startup
        void CreateActions();

        // creates all menus. gets called on startup
        void CreateMenus();

        /* Menus */
        QMenu *_fileMenu;
        QMenu *_imfMenu;

        /* Actions */

        // File Menu
        QAction *_newFileAction;
        QAction *_openFileAction;
        QAction *_saveFileAction;

        // IMF Menu
        QAction *_addTrackFileAction;
        QAction *_newCompositionPlaylistAction;

        /* INSTANCE VARIABLES */
        IMFPackageModel _packageModel;
};

#endif // IMFPACKAGEVIEW_H
