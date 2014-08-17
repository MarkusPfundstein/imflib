#ifndef CPLSEQUENCEVIEW_H
#define CPLSEQUENCEVIEW_H

#include <QWidget>


class CPLSequenceView : public QWidget
{
    Q_OBJECT

    public:
        CPLSequenceView(QWidget *parent = nullptr);
        virtual ~CPLSequenceView();

        QSize minimumSizeHint() const;
        QSize sizeHint() const;
        void paintEvent(QPaintEvent *event);
    protected:
    private:
};

#endif // CPLSEQUENCEVIEW_H