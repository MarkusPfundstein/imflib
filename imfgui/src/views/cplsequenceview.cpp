#include "cplsequenceview.h"

#include <QtGui>
#include <cmath>
#include <iostream>

#include "cplresourcerect.h"
#include "cplsegmentrect.h"
#include "cplsequencerect.h"
#include "imfpackageview.h"
#include "packagetableview.h"

#include "../model/imfcompositionplaylist.h"
#include "../model/cplsegment.h"
#include "../model/cplresource.h"
#include "../model/cplsequence.h"
#include "../model/cplvirtualtrack.h"
#include "../model/imftrack.h"

#include "../utils/uuidgenerator.h"

#include "../application.h"

#include "../model/imfpackage.h"
#include "../model/imfvideotrack.h"
#include "../model/imfaudiotrack.h"

CPLSequenceView::CPLSequenceView(QWidget *_parent)
    :
    QGraphicsView(_parent),
    _compositionPlaylist(nullptr),
    _playlistDuration(0),
    _virtualTrackMap(),
    _videoIcon(),
    _audioIcon(),
    _heightPerTrack(36),
    _lastSegmentRect(nullptr),
    _zoomFactor(8)
{
    setBackgroundRole(QPalette::Shadow);
    setAutoFillBackground(true);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    //QGraphicsScene *gscene = new QGraphicsScene(0, 0, width(), height());
    //setScene(gscene);

    QFileInfo appPath(QCoreApplication::applicationFilePath());
    std::cout << appPath.absoluteDir().absolutePath().toStdString() << std::endl;
    if (_videoIcon.load(appPath.absoluteDir().absolutePath() + tr("/../resources/video.png")) == false) {
        std::cout << "[Error] Video icon not found" << std::endl;
    }
    if (_audioIcon.load(appPath.absoluteDir().absolutePath() + tr("/../resources/audio.png")) == false) {
        std::cout << "[Error] Audio icon not found" << std::endl;
    }
}

CPLSequenceView::~CPLSequenceView()
{
    if (scene()) {
        scene()->clear();
    }
    std::cout << "delete sequence view " << std::endl;
}

QSize CPLSequenceView::minimumSizeHint() const
{
    return QSize(100, 300);
}

QSize CPLSequenceView::sizeHint() const
{
    return QSize(parentWidget()->width(), 256);
}

void CPLSequenceView::CompositionPlaylistChanged(const std::shared_ptr<IMFCompositionPlaylist> &newPlaylist)
{
    _compositionPlaylist = newPlaylist;
    // this is quite some enumerating, so we cache it
    if (_compositionPlaylist) {
        _playlistDuration = _compositionPlaylist->GetDurationInFrames();
        std::cout << "New playlist duration: " << _playlistDuration << " frames, [" << _compositionPlaylist->GetDurationInEditUnits() << "EU]" << std::endl;

        int idx = 0;
        for (const std::shared_ptr<CPLVirtualTrack>& vt : _compositionPlaylist->GetVirtualTracks()) {
            _virtualTrackMap[vt->GetUUID()] = idx;
            std::cout << "Map VirtualTrack: " << vt->GetUUID() << " to index: " << idx << std::endl;
            idx++;
        }
    }

    _lastSegmentRect = nullptr;

    // we also call this if nullptr is passed. to delete scene
    if (scene()) {
        scene()->clear();
    } else {
        setScene(new QGraphicsScene(0, 0, 1600, height()));
    }
    QRectF rect(scene()->sceneRect());
    rect.setWidth(_playlistDuration < 1600 ? 1600 : _playlistDuration + 20);
    scene()->setSceneRect(rect);
    paintScence();
}

void CPLSequenceView::mousePressEvent(QMouseEvent *ev)
{
    std::cout << "mousePressEvent" << std::endl;
    if (ev->button() == Qt::RightButton) {
        std::cout << "Right mouse button called" << std::endl;
        for (QGraphicsItem *cs : scene()->items()) {
            CPLResourceRect *r= dynamic_cast<CPLResourceRect*>(cs);
            if (r && r->boundingRect().contains(ev->pos())) {
                ev->accept();
                IMFPackageView *packageView = dynamic_cast<IMFPackageView*>(parentWidget()->parentWidget());
                if (packageView == nullptr) {
                    return;
                }

                for (QModelIndex modelIndex : packageView->GetPackageTableView().selectionModel()->selectedRows()) {
                    std::shared_ptr<IMFPackageItem> packageItem = packageView->GetPackageModel().IMFPackageInRow(modelIndex.row());
                    if (packageItem &&
                        (packageItem->GetType() == IMFPackageItem::TYPE::VIDEO ||
                         packageItem->GetType() == IMFPackageItem::TYPE::AUDIO)) {
                        std::cout << "Selected " << packageItem->GetFileName() << std::endl;
                        ShowRightClickMenu(ev->pos(),
                                           *r,
                                           std::static_pointer_cast<IMFTrack>(packageItem));
                        break;
                    }
                }
                return;
            }
        }
    }
}

void CPLSequenceView::ShowRightClickMenu(const QPoint &pos,
                                         CPLResourceRect& resourceRect,
                                         const std::shared_ptr<IMFTrack> &track)
{
    QPoint global = mapToGlobal(pos);
    QMenu *popUp = new QMenu(this);

    QAction *insertAfterAction = new QAction(tr("&Append Track"), this);
    QAction *cancel = new QAction(tr("&Cancel"), this);
    popUp->addAction(insertAfterAction);
    popUp->addAction(cancel);

    QAction *execAction = popUp->exec(global);
    if (execAction == insertAfterAction) {
        std::cout << "insert track: " << track->GetFileName() << " after: " << resourceRect.GetResource()->GetTrack()->GetFileName() << std::endl;
        CPLSequenceRect *sequenceRect = dynamic_cast<CPLSequenceRect*>(resourceRect.parentItem());
        if (sequenceRect) {
            std::shared_ptr<CPLResource> newResource(new CPLResource(UUIDGenerator().MakeUUID(),
                                                                     track));
            newResource->SetEntryPoint(0);
            newResource->SetSourceDuration(track->GetDuration());
            newResource->SetRepeatCount(1);
            newResource->SetPlaylistEditRate(_compositionPlaylist->GetEditRate());

            // To-DO: Fix me
            newResource->SetKeyId("");
            newResource->SetHash("");
            newResource->SetSourceEncoding(UUIDGenerator().MakeUUID());

            sequenceRect->GetSequence()->InsertItemAfter(newResource, resourceRect.GetResource());

            CompositionPlaylistChanged(_compositionPlaylist);

            //AppendResource(sequenceRect, newResource);

            //ShiftEverythingRight()

            //AppendResource(sequenceRect, )
        }
    }
}

void CPLSequenceView::paintScence()
{
    if (scene() == nullptr) {
        return;
    }

    int numberTracks = _compositionPlaylist ? _compositionPlaylist->GetVirtualTracks().size() : 4;
    if (numberTracks < 8) {
        numberTracks = 8;
    }
    //int heightPerTrack = 36; //floorf((height() - 10) / numberTracks);
    std::cout << "heightPerTrack: " << _heightPerTrack << std::endl;

    if (_compositionPlaylist) {
        for (const std::shared_ptr<CPLSegment> segment : _compositionPlaylist->GetSegments()) {
            AppendSegment(segment);
        }
    }

    // draw nice lines to separate tracks
    for (int i = 0; i < numberTracks; ++i) {
        int horizontalOffset = i * _heightPerTrack + 1;
        QLine line(0, horizontalOffset, scene()->width(), horizontalOffset);
        scene()->addLine(line, QPen(QColor(0, 0, 0, 188), 1, Qt::DashLine));
    }
}

void CPLSequenceView::AppendSegment(const std::shared_ptr<CPLSegment> &segment)
{
    CPLSegmentRect *lastSegmentRect = _lastSegmentRect;

    int renderOffsetX = 0;
    int lastIndex = 0;

    if (lastSegmentRect) {
        renderOffsetX = lastSegmentRect->boundingRect().x() + lastSegmentRect->boundingRect().width() + 1;
        lastIndex = lastSegmentRect->GetIndex();
    }

    int width = segment->GetDuration();
    std::cout << "RenderOffsetX: " << renderOffsetX << std::endl;
    std::cout << "segmentWidth: " << width << std::endl;
    CPLSegmentRect *newRect = new CPLSegmentRect();
    QRect drawingRect(renderOffsetX, 0, roundf(width / _zoomFactor), height());
    newRect->SetDrawingRect(drawingRect);
    newRect->SetIndex(lastIndex + 1);

    if ((newRect->GetIndex() % 2) == 0) {
        newRect->SetColor(QColor(138, 138, 138));
    } else {
        newRect->SetColor(QColor(107, 107, 107));
    }

    std::cout << "render segment, X: " << newRect->boundingRect().x() << " Y: " << newRect->boundingRect().y();
    std::cout << " W: " << newRect->boundingRect().width() << " H: " << newRect->boundingRect().height() << std::endl;

    for (const std::shared_ptr<CPLSequence> &s : segment->GetItems()) {
        AddSequence(newRect, s);
    }

    scene()->addItem(newRect);

    _lastSegmentRect = newRect;
}

void CPLSequenceView::AddSequence(CPLSegmentRect *segmentRect, const std::shared_ptr<CPLSequence>& sequence)
{
    int trackIdx = _virtualTrackMap[sequence->GetVirtualTrackId()];

    CPLSequenceRect *newRect = new CPLSequenceRect(segmentRect, sequence);
    newRect->SetRenderBackground(true);
    newRect->SetDrawingRect(QRect(segmentRect->boundingRect().x(),
                                  trackIdx * _heightPerTrack,
                                  floorf(sequence->GetDuration() / _zoomFactor),
                                  _heightPerTrack));
    newRect->SetTrackIndex(trackIdx);

    for (const std::shared_ptr<CPLResource> &resource : sequence->GetItems()) {
        CPLResourceRect *resourceRect = AppendResource(newRect, resource);
        newRect->SetLastItem(resourceRect);
    }
}

CPLResourceRect *CPLSequenceView::AppendResource(CPLSequenceRect *sequenceRect, const std::shared_ptr<CPLResource> &resource)
{
    CPLResourceRect *lastResourceRect = sequenceRect->GetLastItem();

    static QColor fillColors[2] = {
        QColor(51, 102, 152),
        QColor(204, 0, 102)
    };

    static QColor shadowColors[2] = {
        QColor(20, 41, 61),
        QColor(82, 0, 41)
    };

    int startX = sequenceRect->boundingRect().x();
    if (lastResourceRect) {
        startX = lastResourceRect->boundingRect().x() + lastResourceRect->boundingRect().width();
    }
    int startY = sequenceRect->GetTrackIndex() * _heightPerTrack;

    int length = roundf(resource->GetNormalizedSourceDuration() / _zoomFactor);

    QImage *image = nullptr;
    IMFPackageItem::TYPE resourceType = resource->GetTrack()->GetType();
    if (resourceType == IMFPackageItem::TYPE::VIDEO) {
        image = &_videoIcon;
    } else if (resourceType == IMFPackageItem::TYPE::AUDIO) {
        image = &_audioIcon; // TO-DO: this stuff should be cached
    }
    if (image == nullptr) {
        std::cout << "[ERROR] Null image" << std::endl;
    }

    int shadowOffsetX = 2;
    int shadowOffsetY = 1;
    QRect drawingRect(startX, startY + 3, length - 1, _heightPerTrack - 5);
    CPLResourceRect *resourceRect = new CPLResourceRect(sequenceRect,
                                                        resource,
                                                        drawingRect,
                                                        fillColors[sequenceRect->GetTrackIndex() % 2],
                                                        shadowColors[sequenceRect->GetTrackIndex() % 2],
                                                        *image);
    resourceRect->SetShadowOffsets(shadowOffsetX, shadowOffsetY);
    std::cout << "drawing rect x: " << drawingRect.x() << " endX: " << sequenceRect->boundingRect().x() + sequenceRect->boundingRect().width() << std::endl;
    if (drawingRect.x() + drawingRect.width() > sequenceRect->boundingRect().x() + sequenceRect->boundingRect().width()) {
        std::cout << "resize sequence rect " << std::endl;
    }
    return resourceRect;
}

#include "../moc_cplsequenceview.cpp"
