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
        { return 1; }

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

        QList<std::shared_ptr<IMFPackageItem>> _data;
};

#endif // IMFPACKAGEMODEL_H
