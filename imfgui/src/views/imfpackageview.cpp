#include "imfpackageview.h"

#include "../application.h"
#include "../model/imfpackage.h"
#include "../model/imfvideotrack.h"
#include "../model/imfaudiotrack.h"
#include "../model/imfcompositionplaylist.h"
#include "../model/imfoutputprofile.h"

#include "../utils/mxfreader.h"
#include "../utils/uuidgenerator.h"

#include "cplsequenceview.h"
#include "cplsequencecontrolview.h"
#include "packagetableview.h"

#include <QtGui>

#include <iostream>

static const QString BaseWindowTitle("ODMedia IMF Suite");

IMFPackageView::IMFPackageView()
    :
    QMainWindow(),
    _fileMenu(nullptr),
    _imfMenu(nullptr),
    _newFileAction(nullptr),
    _openFileAction(nullptr),
    _saveFileAction(nullptr),
    _addTrackFileAction(nullptr),
    _newCompositionPlaylistAction(nullptr),
    _packageModel(),
    _packageTableView(nullptr)
{
    QWidget *mainWidget = new QWidget();
    setCentralWidget(mainWidget);

    QWidget *bottomFiller = new QWidget();
    bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //QWidget *rightTopFiller = new QWidget();
    //rightTopFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    PackageTableView *packageTableView = new PackageTableView();

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

    QHBoxLayout *fileViewLayout = new QHBoxLayout();
    fileViewLayout->addWidget(packageTableView);

    QGroupBox *topFileBox= new QGroupBox();
    topFileBox->setLayout(fileViewLayout);
    mainLayout->addWidget(topFileBox);

    CPLSequenceControlView *controlView = new CPLSequenceControlView(this);
    controlView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(controlView);

    CPLSequenceView *sequenceView = new CPLSequenceView();
    mainLayout->addWidget(sequenceView);

    connect(packageTableView,
            SIGNAL (doubleClicked(const QModelIndex&)),
            this,
            SLOT (TableViewRowSelectedDoubleClick(const QModelIndex&)));

    connect(this,
            SIGNAL(CompositionPlaylistDoubleClick(const std::shared_ptr<IMFCompositionPlaylist>&)),
            sequenceView,
            SLOT(CompositionPlaylistChanged(const std::shared_ptr<IMFCompositionPlaylist> &)));

    // lazy yeah :-)
    _packageTableView = packageTableView;

    CreateActions();
    CreateMenus();
    UpdateMenu();

    mainWidget->setLayout(mainLayout);
    setWindowTitle(BaseWindowTitle + tr(" - New File"));
    setMinimumSize(640, 480);
    resize(1200, 768);
    //showMaximized();

    //NewFile();
}

IMFPackageView::~IMFPackageView()
{
    //dtor
    std::cout << "delete package view" << std::endl;
}

void IMFPackageView::CreateActions()
{
    /* FILE MENU */

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

    /* IMF MENU */

    _addTrackFileAction = new QAction(tr("&Add File"), this);
    _addTrackFileAction->setStatusTip(tr("Adds a file to the project"));
    connect(_addTrackFileAction, SIGNAL(triggered()), this, SLOT(AddTrackFile()));

    _newCompositionPlaylistAction = new QAction(tr("&New Composition Playlist"), this);
    _newCompositionPlaylistAction->setStatusTip(tr("Creates a empty composition playlist"));
    connect(_newCompositionPlaylistAction, SIGNAL(triggered()), this, SLOT(NewCompositionPlaylist()));
}

void IMFPackageView::CreateMenus()
{
    /* FILE MENU */
    _fileMenu = menuBar()->addMenu(tr("&File"));
    _fileMenu->addAction(_newFileAction);
    _fileMenu->addAction(_openFileAction);
    _fileMenu->addAction(_saveFileAction);

    /* IMF MENU */
    _imfMenu = menuBar()->addMenu(tr("&IMF"));
    _imfMenu->addAction(_addTrackFileAction);
    _imfMenu->addAction(_newCompositionPlaylistAction);
}

void IMFPackageView::UpdateMenu()
{
    Application *app = static_cast<Application*>(Application::instance());

    _addTrackFileAction->setEnabled(app->GetWorkingPackage() ? true : false);
    _newCompositionPlaylistAction->setEnabled(app->GetWorkingPackage() ? true : false);
}

void IMFPackageView::TableViewRowSelectedDoubleClick(const QModelIndex &idx)
{
    std::cout << "Row selected: " << idx.row() << std::endl;

    std::shared_ptr<IMFPackageItem> item = _packageModel.IMFPackageInRow(idx.row());
    if (item == nullptr) {
        return;
    }
    if (item->GetType() == IMFPackageItem::TYPE::CPL) {
        std::shared_ptr<IMFCompositionPlaylist> cpl = std::dynamic_pointer_cast<IMFCompositionPlaylist>(item);
        if (cpl) {
            std::cout << "emit signal " << std::endl;
            emit CompositionPlaylistDoubleClick(cpl);
        }
    }
}

void IMFPackageView::NewFile()
{
    std::cout << "new file" << std::endl;

    Application *app = static_cast<Application*>(Application::instance());

    app->SetWorkingPackage(new IMFPackage());

    UpdateMenu();
    _packageModel.Clear();

    setWindowTitle(BaseWindowTitle + tr(" - New File"));

    emit CompositionPlaylistDoubleClick(std::shared_ptr<IMFCompositionPlaylist>(nullptr));
}

void IMFPackageView::OpenFile()
{
    std::cout << "open file" << std::endl;

    Application *app = static_cast<Application*>(Application::instance());

    QString directory = QFileDialog::getExistingDirectory(this,
                                                          tr("Where?"),
                                                          app->Settings()->GetLastOpenDir());
    if (directory.isEmpty()) {
        return;
    }

    app->Settings()->SetLastOpenDir(directory);
    app->Settings()->SaveSettings();

    _packageModel.Clear();

    IMFPackage *newPackage = new IMFPackage();
    app->SetWorkingPackage(newPackage);

    try {
        newPackage->Load(directory.toStdString());

        std::vector<std::shared_ptr<IMFPackageItem>> assets;
        assets.insert(assets.end(), newPackage->GetVideoTracks().begin(), newPackage->GetVideoTracks().end());
        assets.insert(assets.end(), newPackage->GetAudioTracks().begin(), newPackage->GetAudioTracks().end());
        assets.insert(assets.end(), newPackage->GetCompositionPlaylists().begin(), newPackage->GetCompositionPlaylists().end());
        assets.insert(assets.end(), newPackage->GetOutputProfiles().begin(), newPackage->GetOutputProfiles().end());

        for (const std::shared_ptr<IMFPackageItem> &item : assets) {
            if (_packageModel.HasItem(item) == false) {
                _packageModel.AppendItem(item);
            }
        }

        // draw first composition playlist
        if (newPackage->GetCompositionPlaylists().empty() == false) {
            emit CompositionPlaylistDoubleClick(newPackage->GetCompositionPlaylists()[0]);
        }

        UpdateMenu();
        setWindowTitle(BaseWindowTitle + QString::fromStdString(newPackage->GetName()));
    } catch (IMFPackageException &e) {
        QMessageBox::information(this, tr("Sorry"), tr(e.what()));
        NewFile();
    }
}

void IMFPackageView::SaveFile()
{
    std::cout << "save file" << std::endl;

    Application *app = static_cast<Application*>(Application::instance());
    IMFPackage *workingPackage = app->GetWorkingPackage();

    // completely new save. do all the tedious work
    if (workingPackage->GetLocation().empty() || workingPackage->GetName().empty()) {
        QString directory = QFileDialog::getExistingDirectory(this,
                                                              tr("Where?"),
                                                              app->Settings()->GetLastSaveDir());
        if (directory.isEmpty()) {
            return;
        }


        QString name = QInputDialog::getText(this,
                                             tr("Name of IMF package?"),
                                             tr("Name: "));
        if (name.isEmpty()) {
            return;
        }

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
    }

    try {
        workingPackage->CopyTrackFiles();
        workingPackage->Write();
    } catch (IMFPackageException &e) {
        QMessageBox::information(this, tr("Sorry"), tr(e.what()));
    }
}

void IMFPackageView::AddTrackFile()
{
    std::cout << "add track file" << std::endl;

    Application *app = static_cast<Application*>(Application::instance());

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Add Track File"),
                                                    app->Settings()->GetLastOpenedTrackDir(),
                                                    tr("MXF Files (*.mxf)"));
    if (fileName.isEmpty()) {
        return;
    }

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

    try {
        workingPackage->ParseAndAddTrack(fileStdString);
        for (const std::shared_ptr<IMFVideoTrack> &vt : workingPackage->GetVideoTracks()) {
            if (_packageModel.HasItem(vt) == false) {
                _packageModel.AppendItem(vt);
            }
        }
        for (const std::shared_ptr<IMFAudioTrack> &at : workingPackage->GetAudioTracks()) {
            if (_packageModel.HasItem(at) == false) {
                _packageModel.AppendItem(at);
            }
        }
    }  catch (IMFPackageException &e) {
        QMessageBox::information(this, tr("Sorry"), tr(e.what()));
    }
}

void IMFPackageView::NewCompositionPlaylist()
{
    Application *app = static_cast<Application*>(Application::instance());
    IMFPackage *workingPackage = app->GetWorkingPackage();
    if (workingPackage->GetLocation().empty() && workingPackage->GetName().empty()) {
        QMessageBox::information(this, tr("Sorry"), tr("The package is not saved yet. You must save it before you can create a CPL"));
        return;
    }

    QStringList framerates;
    framerates.push_back("24000 1001");
    framerates.push_back("24 1");
    framerates.push_back("25000 1001");
    framerates.push_back("25 1");
    framerates.push_back("30000 1001");
    framerates.push_back("30 1");
    framerates.push_back("48000 1001");
    framerates.push_back("48 1");
    framerates.push_back("50000 1001");
    framerates.push_back("50 1");
    framerates.push_back("60000 1001");
    framerates.push_back("60 1");
    framerates.push_back("120000 1001");
    framerates.push_back("120 1");

    QString frame = QInputDialog::getItem(this,
                                          tr("Have a minute?"),
                                          tr("Edit rate: "),
                                          framerates,
                                          0,
                                          false);
    if (frame.isEmpty()) {
        return;
    }

    RationalNumber er = RationalNumber::FromIMFString(frame.toStdString());
    if (er.denum == 0) {
        return;
    }


    std::string uuid = UUIDGenerator().MakeUUID();
    std::string path = workingPackage->GetFullPath() + "/CPL_" + uuid + ".xml";

    std::shared_ptr<IMFCompositionPlaylist> newPlaylist(new IMFCompositionPlaylist(uuid, path));
    newPlaylist->SetEditRate(er);

    workingPackage->AddCompositionPlaylist(newPlaylist);
    _packageModel.AppendItem(newPlaylist);

    emit CompositionPlaylistDoubleClick(newPlaylist);
}

#include "../moc_imfpackageview.cpp"
