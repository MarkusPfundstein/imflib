#ifndef PACKAGETABLEVIEW_H
#define PACKAGETABLEVIEW_H

#include <QTableView>
#include <memory>

class IMFPackageItem;

class PackageTableView : public QTableView
{
    Q_OBJECT

    public:
        PackageTableView(QWidget *parent = nullptr);
        virtual ~PackageTableView();

        QSize sizeHint() const; /*overwrite*/

        void mousePressEvent(QMouseEvent *event); /*overwrite*/
        void mouseMoveEvent(QMouseEvent *event); /*overwrite*/

        // returns first item that is selected in table view. otherwise nullptr
        std::shared_ptr<IMFPackageItem> GetFirstSelectedItem() const;
    protected:
    private:

        QPoint _startPos;
};

#endif // PACKAGETABLEVIEW_H
