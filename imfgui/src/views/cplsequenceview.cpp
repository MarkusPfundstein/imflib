#include "cplsequenceview.h"

#include <QtGui>
#include <cmath>
#include <iostream>

CPLSequenceView::CPLSequenceView(QWidget *parent)
    : QWidget(parent)
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

void CPLSequenceView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().light().color());
    painter.setBrush(Qt::NoBrush);

    // draw horizontal lines to separate tracks
    int numberTracks = 4;

    int heightPerTrack = floorf(height() / numberTracks);       // lets assume 4 now

    for (int i = 0; i < numberTracks; ++i) {
        std::cout << i << std::endl;
        int horizontalOffset = i * heightPerTrack;

        QLine line(0, horizontalOffset, width(), horizontalOffset);

        /* TESTING */
        if (i == 0) {
            QRect r(1, horizontalOffset + 1, (int)(width() * 0.3f), heightPerTrack - 1);
            painter.fillRect(r, QBrush(QColor(230, 120, 120, 188)));
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
        }

        /* TESTING END */

        painter.drawLine(line);
    }

    painter.end();
}

#include "../moc_cplsequenceview.cpp"
