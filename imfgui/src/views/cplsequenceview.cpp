#include "cplsequenceview.h"

#include <QtGui>
#include <cmath>
#include <iostream>

#include "cplresourcerect.h"

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
    _virtualTrackMap()
{
    setBackgroundRole(QPalette::Shadow);
    setAutoFillBackground(true);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    //QGraphicsScene *gscene = new QGraphicsScene(0, 0, width(), height());
    //setScene(gscene);
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
    // we also call this if nullptr is passed. to delete scene
    if (scene()) {
        scene()->clear();
    } else {
        setScene(new QGraphicsScene(0, 0, width(), height()));
    }
    paintScence();
}

void CPLSequenceView::paintScence()
{
    int numberTracks = _compositionPlaylist ? _compositionPlaylist->GetVirtualTracks().size() : 4;
    if (numberTracks < 8) {
        numberTracks = 8;
    }
    int heightPerTrack = floorf((height() - 10) / numberTracks);

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
        QLine line(0, horizontalOffset, QApplication::desktop()->screenGeometry().width(), horizontalOffset);
        scene()->addLine(line, QPen(QColor(0, 0, 0, 188), 1, Qt::DashLine));
    }
}

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
        RenderResource(*r, seqIdx, offsetX, trackIdx, heightPerTrack, &resourceLength);
        offsetX += resourceLength;
        *sequenceLength += resourceLength;
    }
}

void CPLSequenceView::RenderResource(const CPLResource& resource,
                                     int seqIdx,
                                     int startX,
                                     int trackIdx,
                                     int heightPerTrack,
                                     int *resourceLength)
{
    static QColor brushColors[2] = {
        QColor(51, 102, 152),
        QColor(204, 0, 102)
    };

    static QColor penColors[2] = {
        QColor(20, 41, 61),
        QColor(82, 0, 41)
    };

    int startY = trackIdx * heightPerTrack;

    double widthRatio = (double)_playlistDuration / 1024;

    int length = roundf(resource.GetNormalizedSourceDuration() / widthRatio);

    std::cout << "Drawing Width: " << 1024;
    std::cout << " EndX: " << startX + length << std::endl;

    int shadowOffsetX = 2;
    int shadowOffsetY = 1;

    QRect r;
    r.setCoords(startX, startY + 3, startX + length - 1 - shadowOffsetX, startY + heightPerTrack - 1 - shadowOffsetY);

    CPLResourceRect *resourceRect = new CPLResourceRect(r, penColors[trackIdx % 2], brushColors[trackIdx % 2]);
    resourceRect->SetShadowOffsets(shadowOffsetX, shadowOffsetY);
    scene()->addItem(resourceRect);

    *resourceLength = length;
}

#include "../moc_cplsequenceview.cpp"
