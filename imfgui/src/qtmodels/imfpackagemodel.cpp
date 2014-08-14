#include "imfpackagemodel.h"
#include <iostream>

IMFPackageModel::IMFPackageModel(QObject *parent)
    :
    QAbstractTableModel(parent),
    _data()
{
    //ctor
}

IMFPackageModel::~IMFPackageModel()
{
    //dtor
    std::cout << "delete packagemodel" << std::endl;
}

QVariant IMFPackageModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    std::shared_ptr<IMFPackageItem> item = _data[index.row()];
    switch (index.column()) {
        case 0: return QString("bla");//item.GetFileName();
        default: return QVariant();
    };
}

QVariant IMFPackageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
        case 0: return "Filename";
        default: return QVariant();
    }
}

void IMFPackageModel::AppendItem(std::shared_ptr<IMFPackageItem> item)
{
    beginInsertRows(QModelIndex(), _data.count(), _data.count());
    _data.append(item);
    endInsertRows();
}

void IMFPackageModel::Clear()
{
    //removeRows(0, _data.count(), QModelIndex());
    _data.clear();
    reset();
}
