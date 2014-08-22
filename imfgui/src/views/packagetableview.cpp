#include "packagetableview.h"

#include <iostream>
#include <QtGui>

#include "../model/imfpackageitem.h"
#include "../qtmodels/imfpackagemodel.h"
#include "../qtmodels/sharedpointermimedata.h"
#include "../model/imftrack.h"

PackageTableView::PackageTableView(QWidget *_parent)
    :
    QTableView(_parent)
{
    //ctor
    //setDragEnabled(true);
}

PackageTableView::~PackageTableView()
{
    //dtor
}

void PackageTableView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        _startPos = event->pos();
        std::cout << "set startpos" << std::endl;
    }
    QTableView::mousePressEvent(event);
}

void PackageTableView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - _startPos).manhattanLength();
        if (distance >= QApplication::startDragDistance()) {

            std::shared_ptr<IMFTrack> selectedItem = std::dynamic_pointer_cast<IMFTrack>(GetFirstSelectedItem());
            if (selectedItem) {
                SharedPointerMimeData<IMFTrack> *mimeData = new SharedPointerMimeData<IMFTrack>(selectedItem);

                QDrag *drag = new QDrag(this);
                drag->setMimeData(mimeData);
                drag->exec();
            }
        }
    }

    QTableView::mouseMoveEvent(event);
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
