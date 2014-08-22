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
#include "../qtmodels/sharedpointermimedata.h"

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
    _zoomFactor(8),
    _draggedResourceRect(nullptr)
{
    setBackgroundRole(QPalette::Shadow);
    setAutoFillBackground(true);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    setAcceptDrops(true);

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
    if (_draggedResourceRect) {
        delete _draggedResourceRect;
        _draggedResourceRect = nullptr;
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
    QRectF r(scene()->sceneRect());
    r.setWidth(_playlistDuration < 1600 ? 1600 : _playlistDuration + 20);
    scene()->setSceneRect(r);
    paintScence();
}

void CPLSequenceView::mousePressEvent(QMouseEvent *ev)
{
    QGraphicsView::mousePressEvent(ev);
}

void CPLSequenceView::ShowRightClickMenuOnResource(QPoint position,
                                                    CPLResourceRect& resourceRect)
{
    QPointF scenePos = position;
    QMenu *popUp = new QMenu(this);

    QAction *deleteAction = new QAction(tr("&Delete Selected Item"), this);
    QAction *cancelAction = new QAction(tr("&Cancel"), this);

    popUp->addAction(deleteAction);
    popUp->addAction(cancelAction);

    QAction *execAction = popUp->exec(scenePos.toPoint());
    if (execAction == cancelAction) {
        return;
    } else if (execAction == deleteAction) {
        DeleteResourceAction(resourceRect);
    }
}

void CPLSequenceView::DeleteResourceAction(CPLResourceRect &resourceRect)
{
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              "May I have a second?", "Are you sure you want to delete the resource?",
                                                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) {
        return;
    }

    CPLSequenceRect *sequenceRect = static_cast<CPLSequenceRect*>(resourceRect.parentItem());

    sequenceRect->GetSequence()->DeleteItem(resourceRect.GetResource());

    // if sequence is empty we have to do some clean up work
    if (sequenceRect->GetSequence()->IsEmpty()) {
        // delete from segment
        CPLSegmentRect *segmentRect = static_cast<CPLSegmentRect*>(sequenceRect->parentItem());
        segmentRect->GetItem()->DeleteItem(sequenceRect->GetSequence());

        // delete from virtual track
        std::shared_ptr<CPLVirtualTrack> virtualTrack = *std::next(_compositionPlaylist->GetVirtualTracks().begin(), sequenceRect->GetTrackIndex());
        virtualTrack->DeleteItem(sequenceRect->GetSequence());

        // update composition playlist if necessary
        if (segmentRect->GetItem()->IsEmpty()) {
            _compositionPlaylist->DeleteSegment(segmentRect->GetItem());
        }

        // same for virtual track
        if (virtualTrack->IsEmpty()) {
            _compositionPlaylist->DeleteVirtualTrack(virtualTrack);
        }
    }

    CompositionPlaylistChanged(_compositionPlaylist);
}

void CPLSequenceView::NewSegmentAction(QPointF position)
{
    if (_compositionPlaylist == nullptr) {
        return;
    }
    IMFPackageView *packageView = dynamic_cast<IMFPackageView*>(parentWidget()->parentWidget());
    if (packageView == nullptr) {
        return;
    }

    std::shared_ptr<IMFPackageItem> packageItem = packageView->GetPackageTableView().GetFirstSelectedItem();
    if (packageItem &&
        (packageItem->GetType() == IMFPackageItem::TYPE::VIDEO ||
         packageItem->GetType() == IMFPackageItem::TYPE::AUDIO)) {

        bool newVirtualTrack = false;
        std::shared_ptr<CPLVirtualTrack> virtualTrack = GetVirtualTrackFromY(position.y());
        if (!virtualTrack) {
            newVirtualTrack = true;
            std::cout << "Make new Virtual Track" << std::endl;
            virtualTrack = std::shared_ptr<CPLVirtualTrack>(new CPLVirtualTrack(UUIDGenerator().MakeUUID()));
        } else {
            std::cout << "Got a virtual track" << std::endl;
        }

        // create all necessary stuff. new segment, new resource
        std::shared_ptr<CPLSegment> newSegment(new CPLSegment(UUIDGenerator().MakeUUID()));
        std::shared_ptr<CPLSequence> newSequence(new CPLSequence(UUIDGenerator().MakeUUID()));
        newSequence->SetVirtualTrackId(virtualTrack->GetUUID());

        std::shared_ptr<CPLResource> newResource = CPLResource::StandardResource(std::static_pointer_cast<IMFTrack>(packageItem),
                                                                                 _compositionPlaylist->GetEditRate());
        newSequence->AppendItem(newResource);
        newSegment->AppendItem(newSequence);
        virtualTrack->AppendItem(newSequence);

        _compositionPlaylist->AddSegment(newSegment);
        if (newVirtualTrack) {
            _compositionPlaylist->AddVirtualTrack(virtualTrack);
        }

        CompositionPlaylistChanged(_compositionPlaylist);
    } else if (packageItem) {
        QMessageBox::information(this, tr("Sorry"), tr("Only A/V/TT track can be inserted into playlist"));
    } else {
        QMessageBox::information(this, tr("Sorry"), tr("No track selected"));
    }
}

void CPLSequenceView::dragEnterEvent(QDragEnterEvent *ev)
{
    const SharedPointerMimeData<IMFTrack> *mdata = dynamic_cast<const SharedPointerMimeData<IMFTrack>*>(ev->mimeData());
    if (_compositionPlaylist && mdata) {
        std::shared_ptr<IMFTrack> track = mdata->GetStoredPointer();
        if (track) {
            std::shared_ptr<CPLResource> newResource = CPLResource::StandardResource(track,
                                                                                     _compositionPlaylist->GetEditRate());

            int length = roundf(newResource->GetNormalizedSourceDuration() / _zoomFactor);

            QRect renderRect(0, 0, length - 1, _heightPerTrack - 5);

            QImage *image = nullptr;
            IMFPackageItem::TYPE resourceType = track->GetType();
            if (resourceType == IMFPackageItem::TYPE::VIDEO) {
                image = &_videoIcon;
            } else if (resourceType == IMFPackageItem::TYPE::AUDIO) {
                image = &_audioIcon; // TO-DO: this stuff should be cached
            }
            _draggedResourceRect = new CPLResourceRect(nullptr,
                                                        newResource,
                                                        renderRect,
                                                        QColor(51, 102, 152),
                                                        QColor(20, 41, 61),
                                                        *image);
            _draggedResourceRect->SetShadowOffsets(2, 1);
            scene()->addItem(_draggedResourceRect);

            ev->acceptProposedAction();
        }
    }
}

void CPLSequenceView::dragLeaveEvent(QDragLeaveEvent *ev)
{
    if (_draggedResourceRect) {
        delete _draggedResourceRect;
        _draggedResourceRect = nullptr;
    }
    for (QGraphicsItem *cs : scene()->items()) {
            CPLResourceRect *r= dynamic_cast<CPLResourceRect*>(cs);
            if (r && r != _draggedResourceRect) {
                r->SetDrawAttachAreaRight(false);
                r->SetDrawAttachAreaLeft(false);
                r->update();
            }
    }
    ev->accept();
}

void CPLSequenceView::dropEvent(QDropEvent *ev)
{
    if (_compositionPlaylist && _draggedResourceRect) {
        std::shared_ptr<IMFTrack> track = _draggedResourceRect->GetResource()->GetTrack();
        bool attach = false;
        for (QGraphicsItem *cs : scene()->items()) {
            CPLRenderRect *r= dynamic_cast<CPLRenderRect*>(cs);
            if (r && r != _draggedResourceRect) {
                if (r->GetDrawAttachAreaRight()) {
                    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
                        CPLSegmentRect *segmentRect = dynamic_cast<CPLSegmentRect*>(r);
                        if (segmentRect) {
                            attach = true;
                            InsertSegmentAction(*segmentRect, track, ev->pos().y(), true);
                        }
                    } else {
                        CPLResourceRect *resourceRect = dynamic_cast<CPLResourceRect*>(r);
                        if (resourceRect) {
                            attach = true;
                            InsertResourceAction(*resourceRect, track, true);
                        }
                    }
                } else if (r->GetDrawAttachAreaLeft()) {

                    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
                        CPLSegmentRect *segmentRect = dynamic_cast<CPLSegmentRect*>(r);
                        if (segmentRect) {
                            attach = true;
                            InsertSegmentAction(*segmentRect, track, ev->pos().y(), false);
                        }
                    } else {
                        CPLResourceRect *resourceRect = dynamic_cast<CPLResourceRect*>(r);
                        if (resourceRect) {
                            attach = true;
                            InsertResourceAction(*resourceRect, track, false);
                        }
                    }
                }

                if (attach) {
                    r->SetDrawAttachAreaRight(false);
                    r->SetDrawAttachAreaLeft(false);
                    r->update();
                    break;
                }
            }
        }
        if (_draggedResourceRect) {
            delete _draggedResourceRect;
            _draggedResourceRect = nullptr;
        }


        if (attach) {
            emit CompositionPlaylistChanged(_compositionPlaylist);
        }
    }
}

void CPLSequenceView::dragMoveEvent(QDragMoveEvent *ev)
{
    if (_compositionPlaylist && _draggedResourceRect) {
        QPointF scenePos = mapToScene(ev->pos());
        _draggedResourceRect->setPos(scenePos);
        _draggedResourceRect->update();

        for (QGraphicsItem *cs : scene()->items()) {
            CPLRenderRect *r= dynamic_cast<CPLRenderRect*>(cs);
            if (r && r != _draggedResourceRect) {
                bool checkForContain = true;
                if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
                    CPLSegmentRect *test = dynamic_cast<CPLSegmentRect*>(r);
                    if (test == nullptr) {
                        checkForContain = false;;
                    }
                } else {
                    CPLResourceRect *test = dynamic_cast<CPLResourceRect*>(r);
                    if (test == nullptr) {
                        checkForContain = false;;
                    }
                }
                QRectF sceneRectRight = r->mapRectToScene(r->RightIntersectionRect());
                QRectF sceneRectLeft = r->mapRectToScene(r->LeftIntersectionRect());
                if (checkForContain && sceneRectRight.contains(scenePos)) {
                    r->SetDrawAttachAreaRight(true);
                } else if (checkForContain && sceneRectLeft.contains(scenePos)) {
                    r->SetDrawAttachAreaLeft(true);
                } else {
                    r->SetDrawAttachAreaRight(false);
                    r->SetDrawAttachAreaLeft(false);
                }
                r->update();
            }
        }

        ev->acceptProposedAction();
    }
}

void CPLSequenceView::InsertSegmentAction(const CPLSegmentRect &segmentRect, const std::shared_ptr<IMFTrack> &track, int Y, bool append)
{
    if (_compositionPlaylist == nullptr) {
        return;
    }

    // create new segment and new sequence
    std::shared_ptr<CPLSegment> newSegment(new CPLSegment(UUIDGenerator().MakeUUID()));
    std::shared_ptr<CPLSequence> newSequence(new CPLSequence(UUIDGenerator().MakeUUID()));
    newSegment->AppendItem(newSequence);

    // check for virtual track at Y position. if no make one
    std::shared_ptr<CPLVirtualTrack> virtualTrack = GetVirtualTrackFromY(Y);
    if (!virtualTrack) {
        std::cout << "Make new Virtual Track" << std::endl;
        virtualTrack = std::shared_ptr<CPLVirtualTrack>(new CPLVirtualTrack(UUIDGenerator().MakeUUID()));
        _compositionPlaylist->AddVirtualTrack(virtualTrack);
    }

    // set virtual track id of sequence
    newSequence->SetVirtualTrackId(virtualTrack->GetUUID());
    virtualTrack->AppendItem(newSequence);

    // generate new resource
    std::shared_ptr<CPLResource> newResource = CPLResource::StandardResource(track,
                                                                             _compositionPlaylist->GetEditRate());
    newSequence->AppendItem(newResource);

    if (append) {
        _compositionPlaylist->InsertSegmentAfter(newSegment, segmentRect.GetItem());
    } else {
        _compositionPlaylist->InsertSegmentBefore(newSegment, segmentRect.GetItem());
    }
}

void CPLSequenceView::InsertResourceAction(const CPLResourceRect &resourceRect, const std::shared_ptr<IMFTrack> &track, bool append)
{
    if (_compositionPlaylist == nullptr) {
        return;
    }

    // create plain resource
    // TO-DO: Set some parameters in CPLResource::StandardResource like Hash, KeyId etc after user input
    std::shared_ptr<CPLResource> newResource = CPLResource::StandardResource(track,
                                                                             _compositionPlaylist->GetEditRate());
    if (append) {
        static_cast<CPLSequenceRect*>(resourceRect.parentItem())->GetSequence()->InsertItemAfter(newResource, resourceRect.GetResource());
    } else {
        static_cast<CPLSequenceRect*>(resourceRect.parentItem())->GetSequence()->InsertItemBefore(newResource, resourceRect.GetResource());
    }

}

void CPLSequenceView::AddResourceAction(const CPLSegmentRect& segmentRect, QPointF position)
{
    if (_compositionPlaylist == nullptr) {
        return;
    }

    IMFPackageView *packageView = dynamic_cast<IMFPackageView*>(parentWidget()->parentWidget());
    if (packageView == nullptr) {
        return;
    }

    std::shared_ptr<IMFPackageItem> packageItem = packageView->GetPackageTableView().GetFirstSelectedItem();
    if (packageItem &&
        (packageItem->GetType() == IMFPackageItem::TYPE::VIDEO ||
         packageItem->GetType() == IMFPackageItem::TYPE::AUDIO)) {

        bool newVirtualTrack = false;

        std::shared_ptr<CPLSequence> sequence(nullptr);

        std::shared_ptr<CPLVirtualTrack> virtualTrack = GetVirtualTrackFromY(position.y());
        if (!virtualTrack) {
            newVirtualTrack = true;
            std::cout << "Make new Virtual Track" << std::endl;
            virtualTrack = std::shared_ptr<CPLVirtualTrack>(new CPLVirtualTrack(UUIDGenerator().MakeUUID()));
            _compositionPlaylist->AddVirtualTrack(virtualTrack);
        } else {
            // check if we need to make a sequence
            // -> search in virtual track for sequence
            for (const std::shared_ptr<CPLSequence> &s : segmentRect.GetItem()->GetItems()) {
                sequence = virtualTrack->FindItem(s->GetUUID());
                if (sequence) {
                    break;
                }
            }
        }
        if (sequence == nullptr) {
            std::cout << "Make new sequence" << std::endl;
            sequence = std::shared_ptr<CPLSequence>(new CPLSequence(UUIDGenerator().MakeUUID()));
            segmentRect.GetItem()->AppendItem(sequence);
            virtualTrack->AppendItem(sequence);
            sequence->SetVirtualTrackId(virtualTrack->GetUUID());
        }

        std::shared_ptr<CPLResource> newResource = CPLResource::StandardResource(std::static_pointer_cast<IMFTrack>(packageItem),
                                                                                 _compositionPlaylist->GetEditRate());
        sequence->AppendItem(newResource);

        CompositionPlaylistChanged(_compositionPlaylist);

    } else if (packageItem) {
        QMessageBox::information(this, tr("Sorry"), tr("Only A/V/TT track can be inserted into playlist"));
    } else {
        QMessageBox::information(this, tr("Sorry"), tr("No track selected"));
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
        std::cout << "Render: " << _compositionPlaylist->GetSegments().size() << " Segments " << std::endl;
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

std::shared_ptr<CPLVirtualTrack> CPLSequenceView::GetVirtualTrackFromY(float Y) const
{
    if (_compositionPlaylist == nullptr) {
        return std::shared_ptr<CPLVirtualTrack>(nullptr);
    }
    int index = floorf(Y / _heightPerTrack);
    return _compositionPlaylist->GetVirtualTrackAtIndex(index);
}

void CPLSequenceView::AppendSegment(const std::shared_ptr<CPLSegment> &segment)
{
    CPLSegmentRect *lastSegmentRect = _lastSegmentRect;

    int renderOffsetX = 0;
    int lastIndex = 0;

    if (lastSegmentRect) {
        renderOffsetX = lastSegmentRect->pos().x() + lastSegmentRect->rect().width() + 1;
        lastIndex = lastSegmentRect->GetIndex();
    }

    CPLSegmentRect *newRect = new CPLSegmentRect(nullptr, segment);
    QRect drawingRect(0, 0, roundf(segment->GetDuration() / _zoomFactor), height());
    newRect->SetDrawingRect(drawingRect);
    newRect->setPos(renderOffsetX, 0);
    newRect->SetIndex(lastIndex + 1);

    if ((newRect->GetIndex() % 2) == 0) {
        newRect->SetColor(QColor(138, 138, 138));
    } else {
        newRect->SetColor(QColor(107, 107, 107));
    }

    std::cout << "Render " << segment->GetItems().size() << " sequences" << std::endl;
    scene()->addItem(newRect);
    for (const std::shared_ptr<CPLSequence> &s : segment->GetItems()) {
        AddSequence(newRect, s);
    }

    _lastSegmentRect = newRect;
}

void CPLSequenceView::AddSequence(CPLSegmentRect *segmentRect, const std::shared_ptr<CPLSequence>& sequence)
{
    int trackIdx = _virtualTrackMap[sequence->GetVirtualTrackId()];

    CPLSequenceRect *newRect = new CPLSequenceRect(segmentRect, sequence);
    newRect->SetRenderBackground(true);
    newRect->SetDrawingRect(QRect(0,
                                  0,
                                  floorf(sequence->GetDuration() / _zoomFactor),
                                  _heightPerTrack));
    newRect->setPos(0,
                    trackIdx * _heightPerTrack);
    newRect->SetTrackIndex(trackIdx);


    for (const std::shared_ptr<CPLResource> &resource : sequence->GetItems()) {
        CPLResourceRect *resourceRect = AppendResource(newRect, resource);
        newRect->SetLastItem(resourceRect);
        if (newRect->GetFirstItem() == nullptr) {
            newRect->SetFirstItem(resourceRect);
        }
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

    int shadowOffsetX = 2;
    int shadowOffsetY = 1;

    int startX = sequenceRect->pos().x();
    if (lastResourceRect) {
        startX = lastResourceRect->pos().x() + lastResourceRect->rect().width() + (shadowOffsetX - 1);
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


    QRect drawingRect(0, 0, length - (shadowOffsetX - 1), _heightPerTrack - 5);
    CPLResourceRect *resourceRect = new CPLResourceRect(sequenceRect,
                                                        resource,
                                                        drawingRect,
                                                        fillColors[sequenceRect->GetTrackIndex() % 2],
                                                        shadowColors[sequenceRect->GetTrackIndex() % 2],
                                                        *image);
    resourceRect->SetShadowOffsets(shadowOffsetX, shadowOffsetY);
    resourceRect->setPos(startX, 0);

    connect(resourceRect,
            SIGNAL(RightMouseClickSignal(QPoint, CPLResourceRect&)),
            this,
            SLOT(ShowRightClickMenuOnResource(QPoint, CPLResourceRect&)));

    return resourceRect;
}

#include "../moc_cplsequenceview.cpp"
