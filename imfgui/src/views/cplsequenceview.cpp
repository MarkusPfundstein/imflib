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
    _playlistDuration(0)
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
    }
    update();
}

void CPLSequenceView::paintEvent(QPaintEvent *)
{
    Application *app = static_cast<Application*>(Application::instance());
    IMFPackage *workingPackage = app->GetWorkingPackage();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().light().color());
    painter.setBrush(Qt::NoBrush);

    // draw horizontal lines to separate tracks
    int numberTracks = 4;

    int heightPerTrack = floorf(height() / numberTracks);

    if (_compositionPlaylist) {
        int startX = 0;
        int seqIdx = 0;
        for (const std::shared_ptr<CPLSegment> segment : _compositionPlaylist->GetSegments()) {
            int length;
            RenderSegment(*segment, painter, seqIdx, startX, heightPerTrack, &length);
            startX += length;
            seqIdx++;
        }
    }

    // draw nice lines to separate tracks
    for (int i = 0; i < numberTracks; ++i) {
        int horizontalOffset = i * heightPerTrack;

        /* TESTING */
        if (_compositionPlaylist) {
            //std::vector<std::shared_ptr<CPLVirtualTrack>> virtualTracks = _compositionPlaylist->GetVirtualTracks();

            /*if (i == 0) {
                CPLResourceRect r(1, horizontalOffset + 1, width() * 0.3f, heightPerTrack - 1, 230, 120, 120, 188);
                r.Draw(painter);
            } else if (i == 1) {
                QRect r(1, horizontalOffset + 1, (int)(width() * 0.3f), heightPerTrack - 1);
                painter.fillRect(r, QBrush(QColor(120, 230, 120, 188)));
            } else if (i == 2) {
                //std::cout << "one draw" << std::endl;
                QRect r(width() * 0.3f, horizontalOffset + 1, (width() * 0.8f) - width() * 0.3f, heightPerTrack - 1);
                painter.fillRect(r, QBrush(QColor(120, 120, 230, 188)));
            } else if (i == 3) {
                QRect r(width() * 0.3f, horizontalOffset + 1, (width() * 0.8f) - width() * 0.3f, heightPerTrack - 1);
                painter.fillRect(r, QBrush(QColor(230, 230, 120, 188)));
            }*/
        }

        /* TESTING END */

        QLine line(0, horizontalOffset, width(), horizontalOffset);
        painter.drawLine(line);
    }

    painter.end();
}

void CPLSequenceView::RenderSegment(const CPLSegment& segment, QPainter &painter, int seqIdx, int startX, int heightPerTrack, int *sequenceLength)
{
    *sequenceLength = 0;
    // render all sequences on their virtual tracks (at the moment under each other)
    int i = 0;
    std::cout << "Render Segment. StartX: " << startX << std::endl;
    for (const std::shared_ptr<CPLSequence> &s : segment.GetSequences()) {
        int length;
        RenderSequence(*s, painter, seqIdx, startX, i, heightPerTrack, &length);
        i++;
        *sequenceLength = std::max(*sequenceLength, length);
    }
}

void CPLSequenceView::RenderSequence(const CPLSequence &sequence, QPainter &painter, int seqIdx, int startX, int trackIdx, int heightPerTrack, int *sequenceLength)
{
    static QColor colors[4] = {
        QColor(120, 230, 120, 188),
        QColor(230, 230, 120, 188),
        QColor(120, 230, 230, 188),
        QColor(230, 230, 230, 188)
    };

    int horizontalOffset = trackIdx * heightPerTrack;

    double widthRatio = (double)_playlistDuration / width();

    int length = roundf(sequence.GetDuration() / widthRatio);

    int endX = startX + length;

    std::cout << "widthRatio: " << widthRatio << std::endl;
    std::cout << "horiz Off: " << horizontalOffset << std::endl;
    std::cout << "width: " << width() << " endX: " << endX << " heightPerTrack: " << heightPerTrack << std::endl;
    std::cout << "Render Sequence. StartX: " << startX << " length: " << length << " trackIdx: " << trackIdx << std::endl;
    //CPLResourceRect r(startX, horizontalOffset + 1, endX, heightPerTrack - 1, 230, 120, 120, 188);
    //r.Draw(painter);
    QRect r(startX, horizontalOffset + 1, length, heightPerTrack - 1);
    painter.fillRect(r, QBrush(colors[seqIdx % 4]));

    *sequenceLength = length;
}

#include "../moc_cplsequenceview.cpp"
