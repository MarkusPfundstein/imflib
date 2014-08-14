#include "imfpackageview.h"

#include "../application.h"
#include "../model/imfpackage.h"

#include <QWidget>
#include <QBoxLayout>
#include <QMenuBar>
#include <QFileDialog>
#include <QFileInfo>

#include <iostream>

IMFPackageView::IMFPackageView()
{
    QWidget *mainWidget = new QWidget();
    setCentralWidget(mainWidget);

    QWidget *topFiller = new QWidget();
    topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget *bottomFiller = new QWidget;
    bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(5);
    layout->addWidget(topFiller);
    layout->addWidget(bottomFiller);
    mainWidget->setLayout(layout);

    CreateActions();
    CreateMenus();

    setWindowTitle(tr("ODMedia IMF Gui"));
    setMinimumSize(160, 160);
    resize(640, 480);
}

IMFPackageView::~IMFPackageView()
{
    //dtor
}

void IMFPackageView::CreateActions()
{
    _newFileAction = new QAction(tr("&New"), this);
    _newFileAction->setShortcuts(QKeySequence::New);
    _newFileAction->setStatusTip(tr("Create a new IMF file"));
    connect(_newFileAction, SIGNAL(triggered()), this, SLOT(NewFile()));

    _openFileAction = new QAction(tr("&Open"), this);
    _openFileAction->setShortcuts(QKeySequence::Open);
    _openFileAction->setStatusTip(tr("Open a IMF file"));
    connect(_openFileAction, SIGNAL(triggered()), this, SLOT(OpenFile()));

    _saveFileAction = new QAction(tr("&Save"), this);
    _saveFileAction->setShortcuts(QKeySequence::Save);
    _saveFileAction->setStatusTip(tr("Save a File"));
    connect(_saveFileAction, SIGNAL(triggered()), this, SLOT(SaveFile()));

    _addTrackFileAction = new QAction(tr("&Add File"), this);
    _addTrackFileAction->setStatusTip(tr("Adds a file to the project"));
    connect(_addTrackFileAction, SIGNAL(triggered()), this, SLOT(AddTrackFile()));
}

void IMFPackageView::CreateMenus()
{
    _fileMenu = menuBar()->addMenu(tr("&File"));
    _fileMenu->addAction(_newFileAction);
    _fileMenu->addAction(_openFileAction);
    _fileMenu->addAction(_saveFileAction);

    _imfMenu = menuBar()->addMenu(tr("&IMF"));
    _imfMenu->addAction(_addTrackFileAction);
}

void IMFPackageView::NewFile()
{
    std::cout << "new file" << std::endl;

    Application *app = static_cast<Application*>(Application::instance());

    app->SetWorkingPackage(new IMFPackage());
}

void IMFPackageView::OpenFile()
{
    std::cout << "open file" << std::endl;
}

void IMFPackageView::SaveFile()
{
    std::cout << "save file" << std::endl;
}

void IMFPackageView::AddTrackFile()
{
    std::cout << "add track file" << std::endl;

    Application *app = static_cast<Application*>(Application::instance());

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Add Track File"),
                                                    app->Settings()->GetLastOpenedTrackDir(),
                                                    tr("MXF Files (*.mxf)"));

    QFileInfo fileInfo(fileName);
    app->Settings()->SetLastOpenedTrackDir(fileInfo.absolutePath());
    app->Settings()->SaveSettings();
}

#include "../moc_imfpackageview.cpp"
