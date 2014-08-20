#include "cplsequenceview.h"

#include <QtGui>
#include <cmath>
#include <iostream>

#include "cplresourcerect.h"
#include "cplsegmentrect.h"
#include "cplsequencerect.h"

#include "../model/imfcompositionplaylist.h"
#include "../model/cplsegment.h"
#include "../model/cplresource.h"
#include "../model/cplsequence.h"
#include "../model/cplvirtualtrack.h"

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
    //dtor
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

    for (const std::shared_ptr<CPLSequence> &s : segment->GetSequences()) {
        AddSequence(newRect, s);
    }

    scene()->addItem(newRect);

    _lastSegmentRect = newRect;
}

void CPLSequenceView::AddSequence(CPLSegmentRect *segmentRect, const std::shared_ptr<CPLSequence>& sequence)
{
    int trackIdx = _virtualTrackMap[sequence->GetVirtualTrackId()];

    CPLSequenceRect *newRect = new CPLSequenceRect(segmentRect);
    newRect->SetDrawingRect(QRect(segmentRect->boundingRect().x(),
                                  trackIdx * _heightPerTrack,
                                  floorf(sequence->GetDuration() / _zoomFactor),
                                  _heightPerTrack));
    newRect->SetTrackIndex(trackIdx);

    for (const std::shared_ptr<CPLResource> &resource : sequence->GetResources()) {
        AppendResource(newRect, resource);
    }
}

void CPLSequenceView::AppendResource(CPLSequenceRect *sequenceRect, const std::shared_ptr<CPLResource> &resource)
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
    QRect drawingRect(startX, startY, length, _heightPerTrack);
    CPLResourceRect *resourceRect = new CPLResourceRect(sequenceRect,
                                                        resource,
                                                        drawingRect,
                                                        fillColors[sequenceRect->GetTrackIndex() % 2],
                                                        shadowColors[sequenceRect->GetTrackIndex() % 2],
                                                        *image);
    resourceRect->SetShadowOffsets(shadowOffsetX, shadowOffsetY);

    sequenceRect->SetLastItem(resourceRect);
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
/*

    std::vector<int> _separatorOffsets;
    if (_compositionPlaylist) {
        int startX = 0;
        int seqIdx = 0;
        _separatorOffsets.push_back(0);
        for (const std::shared_ptr<CPLSegment> segment : _compositionPlaylist->GetSegments()) {
            int length;
            RenderSegment(*segment, seqIdx, startX + 1, heightPerTrack, &length);

            _separatorOffsets.push_back(startX + length + 1);

            startX += length;
            seqIdx++;
        }
        // render segment lines
        for (int sx: _separatorOffsets) {
             // render segment line
            QLine line(sx, 0, sx, scene()->height());
            scene()->addLine(line, QPen(QColor(0, 0, 0, 128), 2));
        }
    }

    // draw nice lines to separate tracks
    for (int i = 0; i < numberTracks; ++i) {
        int horizontalOffset = i * heightPerTrack + 1;
        QLine line(0, horizontalOffset, scene()->width(), horizontalOffset);
        scene()->addLine(line, QPen(QColor(0, 0, 0, 188), 1, Qt::DashLine));
    }
}
*/

void CPLSequenceView::RenderSegment(const CPLSegment& segment,
                                    int seqIdx,
                                    int startX,
                                    int heightPerTrack,
                                    int *sequenceLength)
{
    *sequenceLength = 0;
    // render all sequences on their virtual tracks
    for (const std::shared_ptr<CPLSequence> &s : segment.GetSequences()) {
        int length;
        int i = _virtualTrackMap[s->GetVirtualTrackId()];

        RenderSequence(*s, seqIdx, startX, i, heightPerTrack, &length);
        *sequenceLength = std::max(*sequenceLength, length);
    }
}

void CPLSequenceView::RenderSequence(const CPLSequence &sequence,
                                     int seqIdx,
                                     int startX,
                                     int trackIdx,
                                     int heightPerTrack,
                                     int *sequenceLength)
{
    *sequenceLength = 0;
    int offsetX = startX;
    for (const std::shared_ptr<CPLResource>& r : sequence.GetResources()) {
        int resourceLength = 0;
        RenderResource(r, seqIdx, offsetX, trackIdx, heightPerTrack, &resourceLength);
        offsetX += resourceLength;
        *sequenceLength += resourceLength;
    }
}

void CPLSequenceView::mousePressEvent(QMouseEvent *ev)
{
    std::cout << "mousePressEvent" << std::endl;
    if (ev->button() == Qt::RightButton) {
        std::cout << "Right mouse button called" << std::endl;
        for (QGraphicsItem *cs : scene()->items()) {
            CPLResourceRect *r= dynamic_cast<CPLResourceRect*>(cs);
            if (r && r->boundingRect().contains(ev->pos())) {
                std::cout << "on child" << std::endl;
                //onChild = true;
                return;
            }
        }
    }

    QGraphicsView::mousePressEvent(ev);
}

void CPLSequenceView::RenderResource(const std::shared_ptr<CPLResource>& resource,
                                     int seqIdx,
                                     int startX,
                                     int trackIdx,
                                     int heightPerTrack,
                                     int *resourceLength)
{
    static QColor fillColors[2] = {
        QColor(51, 102, 152),
        QColor(204, 0, 102)
    };

    static QColor shadowColors[2] = {
        QColor(20, 41, 61),
        QColor(82, 0, 41)
    };

    int startY = trackIdx * heightPerTrack;

    double zoomFactor = 9.0;

    int length = roundf(resource->GetNormalizedSourceDuration() / zoomFactor);

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
    QRect r;
    r.setCoords(startX, startY + 3, startX + length - 1 - shadowOffsetX, startY + heightPerTrack - 1 - shadowOffsetY);

    CPLResourceRect *resourceRect = nullptr;//new CPLResourceRect(resource, r, fillColors[trackIdx % 2], shadowColors[trackIdx % 2], *image);
    resourceRect->SetShadowOffsets(shadowOffsetX, shadowOffsetY);
    //scene()->addItem(resourceRect);

    *resourceLength = length;
}

#include "../moc_cplsequenceview.cpp"
