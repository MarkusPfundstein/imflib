#include "packagetableview.h"

#include <QtGui>

PackageTableView::PackageTableView(QWidget *parent)
    :
    QTableView(parent)
{
    //ctor
}

PackageTableView::~PackageTableView()
{
    //dtor
}

QSize PackageTableView::sizeHint() const
{
    return QSize(parentWidget()->width(), 300);
}

#include "../moc_packagetableview.cpp"
