#include "imfpackagemodel.h"
#include <iostream>

#include <QFileInfo>
#include <cmath>
#include "../model/imftrack.h"

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
    bool trackItem = item->GetType() == IMFPackageItem::TYPE::AUDIO || IMFPackageItem::TYPE::VIDEO;
    switch (index.column()) {
        case 0: return GetDisplayFileName(*item);
        case 1: return QString::fromStdString(item->TypeString());
        case 2: return trackItem ? GetItemBitDepth(*item) : QVariant();
        case 3: return trackItem ? GetItemDuration(*item) : QVariant();
        case 4: return trackItem ? GetItemDurationString(*item) : QVariant();
        default: return QVariant();
    };
}

QString IMFPackageModel::GetItemBitDepth(const IMFPackageItem &item) const
{
    try {
        const IMFTrack& track = dynamic_cast<const IMFTrack&>(item);
        return QString::number(track.GetBits());
    } catch (std::bad_cast e) {
        return QString(-1);
    }
}

QString IMFPackageModel::GetItemDuration(const IMFPackageItem &item) const
{
    try {
        const IMFTrack& track = dynamic_cast<const IMFTrack&>(item);
        return QString::number(track.GetDuration());
    } catch (std::bad_cast e) {
        return QString("-1");
    }
}

QString IMFPackageModel::GetItemDurationString(const IMFPackageItem &item) const
{
    try {
        const IMFTrack& track = dynamic_cast<const IMFTrack&>(item);
        float duration = track.GetDuration() / track.GetEditRate().AsFloat();
        if (duration <= 0.0f) {
            return QString("N/A");
        }
        int temp = (int)duration;
        QString res;
        int seconds = temp % 60;
        temp  /= 60;
        int minutes = temp % 60;
        temp  /= 60;
        int hours = temp  % 24;
        //std::cout << duration << std::endl;
        float mantissa = duration - floorf(duration);
        //std::cout << mantissa << std::endl;

        return res.sprintf("%02d:%02d:%02d.%02d", hours, minutes, seconds, (int)((floorf(mantissa * 100) / 100) * 100));
    } catch (std::bad_cast e) {
        return QString("00:00:00.00");
    }
}

QString IMFPackageModel::GetDisplayFileName(const IMFPackageItem &item) const
{
    return QString::fromStdString(item.GetFileName());
}

QVariant IMFPackageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
        case 0: return "Filename";
        case 1: return "Type";
        case 2: return "BitDepth";
        case 3: return "Frames";
        case 4: return "Duration";
        default: return QVariant();
    }
}

void IMFPackageModel::AppendItem(std::shared_ptr<IMFPackageItem> item)
{
    beginInsertRows(QModelIndex(), _data.count(), _data.count());
    _data.append(item);
    endInsertRows();
}


bool IMFPackageModel::HasItem(std::shared_ptr<IMFPackageItem> item) const
{
    for (const std::shared_ptr<IMFPackageItem>& item2 : _data) {
        if (item->GetUUID() == item2->GetUUID()) return true;
    }
    return false;
}

void IMFPackageModel::Clear()
{
    //removeRows(0, _data.count(), QModelIndex());
    _data.clear();
    reset();
}
