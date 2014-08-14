#ifndef IMFPACKAGEVIEW_H
#define IMFPACKAGEVIEW_H

#include <QMainWindow>
#include <QMenu>
//#include <QObject>

class IMFPackageView : public QMainWindow
{
    Q_OBJECT

    public:
        IMFPackageView();
        virtual ~IMFPackageView();
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
};

#endif // IMFPACKAGEVIEW_H
