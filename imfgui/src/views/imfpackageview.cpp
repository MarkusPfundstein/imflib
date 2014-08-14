#include "imfpackageview.hpp"

#include <QWidget>
#include <QBoxLayout>
#include <QMenuBar>

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

}

void IMFPackageView::CreateMenus()
{
    _fileMenu = menuBar()->addMenu(tr("&File"));
    _fileMenu->addAction(_newFileAction);
    _fileMenu->addAction(_openFileAction);
    _fileMenu->addAction(_saveFileAction);
}

void IMFPackageView::NewFile()
{
    std::cout << "new file" << std::endl;
}

void IMFPackageView::OpenFile()
{
    std::cout << "open file" << std::endl;
}

void IMFPackageView::SaveFile()
{
    std::cout << "save file" << std::endl;
}
#include "../moc_imfpackageview.cpp"
