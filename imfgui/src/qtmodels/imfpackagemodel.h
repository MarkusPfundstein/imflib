#ifndef IMFPACKAGEMODEL_H
#define IMFPACKAGEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <memory>
#include "../model/imfpackageitem.h"

class IMFPackageModel : public QAbstractTableModel
{
    public:
        IMFPackageModel(QObject *parent = nullptr);
        virtual ~IMFPackageModel();

        // OVERWRITE
        int rowCount(const QModelIndex &) const
        { return _data.count(); }

        // OVERWRITE
        int columnCount(const QModelIndex &) const
        { return 5; }

        // OVERWRITE
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;

        // OVERWRITE
        QVariant data(const QModelIndex &index, int role) const;

        // appends item to the end of the table view
        void AppendItem(std::shared_ptr<IMFPackageItem> item);

        // clears data
        void Clear();

    protected:
    private:
        // tries to cast item to IMFTrack and returns duration in frames as string
        QString GetItemDuration(const IMFPackageItem &item) const;

        // returns duration as time string
        QString GetItemDurationString(const IMFPackageItem &item) const;

        // returns bit depth of item as string
        QString GetItemBitDepth(const IMFPackageItem &item) const;

        // returns file name from file path from IMFPackageItem
        QString GetDisplayFileName(const IMFPackageItem &item) const;

        // model data
        QList<std::shared_ptr<IMFPackageItem>> _data;
};

#endif // IMFPACKAGEMODEL_H
