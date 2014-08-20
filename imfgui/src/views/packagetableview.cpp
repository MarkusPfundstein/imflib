#include "packagetableview.h"

#include <QtGui>

#include "../model/imfpackageitem.h"
#include "../qtmodels/imfpackagemodel.h"

PackageTableView::PackageTableView(QWidget *_parent)
    :
    QTableView(_parent)
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

std::shared_ptr<IMFPackageItem> PackageTableView::GetFirstSelectedItem() const
{
    for (QModelIndex modelIndex : selectionModel()->selectedRows()) {
        return static_cast<IMFPackageModel*>(model())->IMFPackageInRow(modelIndex.row());
    }
    return std::shared_ptr<IMFPackageItem>(nullptr);
}

#include "../moc_packagetableview.cpp"
