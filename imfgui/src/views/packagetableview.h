#ifndef PACKAGETABLEVIEW_H
#define PACKAGETABLEVIEW_H

#include <QTableView>


class PackageTableView : public QTableView
{
    Q_OBJECT

    public:
        PackageTableView(QWidget *parent = nullptr);
        virtual ~PackageTableView();

        QSize sizeHint() const;
    protected:
    private:
};

#endif // PACKAGETABLEVIEW_H
