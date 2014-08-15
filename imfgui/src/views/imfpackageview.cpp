#include "imfpackageview.h"

#include "../application.h"
#include "../model/imfpackage.h"
#include "../model/imfvideotrack.h"
#include "../model/imfaudiotrack.h"

#include "../utils/mxfreader.h"
#include "../utils/uuidgenerator.h"

#include <QWidget>
#include <QGridLayout>
#include <QFormLayout>
#include <QInputDialog>
#include <QHeaderView>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTableView>

#include <iostream>

static const QString BaseWindowTitle("ODMedia IMF Suite");

IMFPackageView::IMFPackageView()
    :
    QMainWindow()
{
    QWidget *mainWidget = new QWidget();
    setCentralWidget(mainWidget);

    QWidget *bottomFiller = new QWidget();
    bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget *rightTopFiller = new QWidget();
    rightTopFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QTableView *packageTableView = new QTableView();

    packageTableView->setModel(&_packageModel);

    //packageTableView->horizontalHeader()->setStretchLastSection(true);
    packageTableView->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    packageTableView->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    packageTableView->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    packageTableView->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);
    packageTableView->horizontalHeader()->setResizeMode(3, QHeaderView::ResizeToContents);
    packageTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    packageTableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // adjust tableview after new data gets inserted so that column width
    // fits to new data.

    //connect(&_packageModel,
      //      SIGNAL(rowsInserted(const QModelIndex&, int, int)),
        //    packageTableView,
          //  SLOT(resizeColumnsToContents()));


    QVBoxLayout *mainLayout = new QVBoxLayout();

    QGroupBox *topFileBox= new QGroupBox();


    QHBoxLayout *fileViewLayout = new QHBoxLayout();
    fileViewLayout->addWidget(packageTableView);
    fileViewLayout->addWidget(rightTopFiller);
    topFileBox->setLayout(fileViewLayout);

    mainLayout->addWidget(topFileBox);
    mainLayout->addWidget(bottomFiller);

    CreateActions();
    CreateMenus();
    UpdateMenu();

    mainWidget->setLayout(mainLayout);
    setWindowTitle(BaseWindowTitle + tr(" - New File"));
    setMinimumSize(640, 480);
    resize(1024, 768);

    NewFile();
}

IMFPackageView::~IMFPackageView()
{
    //dtor
    std::cout << "delete package view" << std::endl;
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

void IMFPackageView::UpdateMenu()
{
    Application *app = static_cast<Application*>(Application::instance());

    _addTrackFileAction->setEnabled(app->GetWorkingPackage() ? true : false);
}

void IMFPackageView::NewFile()
{
    std::cout << "new file" << std::endl;

    Application *app = static_cast<Application*>(Application::instance());

    app->SetWorkingPackage(new IMFPackage());

    UpdateMenu();
    _packageModel.Clear();

    setWindowTitle(BaseWindowTitle + tr(" - New File"));
}

void IMFPackageView::OpenFile()
{
    std::cout << "open file" << std::endl;

    UpdateMenu();
}

void IMFPackageView::SaveFile()
{
    std::cout << "save file" << std::endl;

    QString finalLocation;
    QString directory;
    QString name;

    Application *app = static_cast<Application*>(Application::instance());

    IMFPackage *workingPackage = app->GetWorkingPackage();

    // completely new save. do all the tedious work
    if (workingPackage->GetLocation().empty() && workingPackage->GetName().empty()) {
        directory = QFileDialog::getExistingDirectory(this,
                                                      tr("Where?"),
                                                      app->Settings()->GetLastSaveDir());

        name = QInputDialog::getText(this,
                                     tr("Name of IMF package?"),
                                     tr("Name: "));

        QString finalLocation = directory + tr("/") + name;

        if (QDir(finalLocation).exists()) {
            QMessageBox::information(this,
                                     tr("Sorry"),
                                     tr("Directory with that name exists already at location"));
            return;
        }

        if (QDir().mkdir(finalLocation) == false) {
            QMessageBox::information(this,
                                     tr("Sorry"),
                                     tr("Error creating directory (rights?)"));
            return;
        }

        UUIDGenerator uuidGenerator;

        workingPackage->SetLocation(directory.toStdString());
        workingPackage->SetName(name.toStdString());
        workingPackage->SetUUID(uuidGenerator.MakeUUID());
        setWindowTitle(BaseWindowTitle + tr(" - ") + name);

        app->Settings()->SetLastSaveDir(directory);
        app->Settings()->SaveSettings();
    } else {
        directory = QString::fromStdString(workingPackage->GetLocation());
        name = QString::fromStdString(workingPackage->GetName());
    }

    workingPackage->Write();
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

    IMFPackage *workingPackage = app->GetWorkingPackage();

    std::string fileStdString = fileName.toStdString();
    if (workingPackage->HasTrackFile(fileStdString)) {
        QMessageBox::information(this,
                                 tr("Sorry"),
                                 tr("Track file already in package"));
        return;
    }

    MXFReader mxfReader(fileStdString);

    MXFReader::ESSENCE_TYPE essenceType = mxfReader.GetEssenceType();

    if (essenceType == MXFReader::ESSENCE_TYPE::VIDEO) {
        std::shared_ptr<IMFVideoTrack> videoTrack(new IMFVideoTrack(fileStdString));

        // parse header to get metadata for videotrack
        try {
            mxfReader.ParseMetadata(videoTrack);
        } catch (MXFReaderException &ex) {
            QMessageBox::information(this,
                                 tr("Sorry"),
                                 tr(ex.what()));
            return;
        }

        // add to abstract working model
        workingPackage->AddVideoTrack(videoTrack);

        // add to display model
        _packageModel.AppendItem(videoTrack);
    } else if (essenceType == MXFReader::ESSENCE_TYPE::AUDIO) {
        std::shared_ptr<IMFAudioTrack> audioTrack(new IMFAudioTrack(fileStdString));

        // parse header to get metadata for audiotrack
        try {
            mxfReader.ParseMetadata(audioTrack);
        } catch (MXFReaderException &ex) {
            QMessageBox::information(this,
                                 tr("Sorry"),
                                 tr(ex.what()));
            return;
        }

        // add to abstract working model
        workingPackage->AddAudioTrack(audioTrack);

        // add to display model
        _packageModel.AppendItem(audioTrack);


    } else {
        QMessageBox::information(this,
                                 tr("Sorry"),
                                 tr("Invalid essence type (not JPEG2000 or PCM24bit)"));
    }
}

#include "../moc_imfpackageview.cpp"
