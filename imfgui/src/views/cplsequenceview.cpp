#include "cplsequenceview.h"

#include <QtGui>

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

#include "../moc_cplsequenceview.cpp"
