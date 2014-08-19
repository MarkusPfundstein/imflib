#include "cplsequenceview.h"

#include <QtGui>
#include <cmath>
#include <iostream>

#include "../drawing/cplresourcerect.h"

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
    QWidget(_parent),
    _compositionPlaylist(nullptr),
    _playlistDuration(0),
    _virtualTrackMap()
{
    setBackgroundRole(QPalette::Shadow);
    setAutoFillBackground(true);
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
    update();
}

void CPLSequenceView::paintEvent(QPaintEvent *)
{
    //Application *app = static_cast<Application*>(Application::instance());
    //IMFPackage *workingPackage = app->GetWorkingPackage();

    for (QObject *child : children()) {
        CPLResourceRect *rect = dynamic_cast<CPLResourceRect*>(child);
        if (rect) {
            delete rect;
        }
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setBrush(Qt::NoBrush);

    int numberTracks = _compositionPlaylist ? _compositionPlaylist->GetVirtualTracks().size() : 4;
    int heightPerTrack = floorf(height() / numberTracks);

    std::vector<int> _separatorOffsets;
    if (_compositionPlaylist) {
        int startX = 0;
        int seqIdx = 0;
        _separatorOffsets.push_back(0);
        for (const std::shared_ptr<CPLSegment> segment : _compositionPlaylist->GetSegments()) {
            int length;
            RenderSegment(*segment, painter, seqIdx, startX, heightPerTrack, &length);

            _separatorOffsets.push_back(startX + length);

            startX += length;
            seqIdx++;
        }
        // render segment lines
        for (int sx: _separatorOffsets) {
             // render segment line
            QLine line(sx, 0, sx, height());
            painter.setPen(QPen(QColor(0, 0, 0), 2));
            painter.drawLine(line);
        }
    }

    // draw nice lines to separate tracks
    for (int i = 1; i < numberTracks; ++i) {
        int horizontalOffset = i * heightPerTrack;
        QLine line(0, horizontalOffset, width(), horizontalOffset);
        painter.setPen(QPen(QColor(188, 188, 188)));
        painter.drawLine(line);
    }

    painter.end();
}

void CPLSequenceView::RenderSegment(const CPLSegment& segment,
                                    QPainter &painter,
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
        RenderSequence(*s, painter, seqIdx, startX, i, heightPerTrack, &length);
        *sequenceLength = std::max(*sequenceLength, length);
    }
}

void CPLSequenceView::RenderSequence(const CPLSequence &sequence,
                                     QPainter &painter,
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
        RenderResource(*r, painter, seqIdx, offsetX, trackIdx, heightPerTrack, &resourceLength);
        offsetX += resourceLength;
        *sequenceLength += resourceLength;
    }
}

void CPLSequenceView::ResourceRectGotSelected(CPLResourceRect *target)
{
    bool doUpdate = false;
    for (QObject *child : children()) {
        CPLResourceRect *childRect = dynamic_cast<CPLResourceRect*>(child);
        if (childRect) {
            doUpdate = true;
            childRect->SetSelected(childRect == target);
        }
    }
    if (doUpdate) {
        update();
    }
}

void CPLSequenceView::RenderResource(const CPLResource& resource,
                                     QPainter &painter,
                                     int seqIdx,
                                     int startX,
                                     int trackIdx,
                                     int heightPerTrack,
                                     int *resourceLength)
{
    static QColor colors[4] = {
        QColor(120, 230, 120, 188),
        QColor(230, 230, 120, 188),
        QColor(120, 120, 230, 188),
        QColor(230, 120, 230, 188)
    };

    int startY = trackIdx * heightPerTrack;

    double widthRatio = (double)_playlistDuration / width();

    int length = roundf(resource.GetNormalizedSourceDuration() / widthRatio);

    //std::cout << "Widget Width: " << width();
    //std::cout << " EndX: " << startX + length << std::endl;

    CPLResourceRect *r = new CPLResourceRect(this,
                                             QPoint(startX, startY + 1),
                                             QPoint(startX + length, startY + heightPerTrack),
                                             colors[seqIdx % 4]);
    r->Draw(painter);

    connect(r,
            SIGNAL(IGotSelected(CPLResourceRect*)),
            this,
            SLOT(ResourceRectGotSelected(CPLResourceRect*)));

    *resourceLength = length;
}

#include "../moc_cplsequenceview.cpp"
