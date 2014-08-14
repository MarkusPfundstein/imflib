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
        void NewFile();
        void OpenFile();
        void SaveFile();

    private:
        void CreateActions();
        void CreateMenus();

        QMenu *_fileMenu;

        QAction *_newFileAction;
        QAction *_openFileAction;
        QAction *_saveFileAction;
};

#endif // IMFPACKAGEVIEW_H
