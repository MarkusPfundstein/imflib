#ifndef CPLSEQUENCECONTROLVIEW_H
#define CPLSEQUENCECONTROLVIEW_H

#include <QWidget>

class CPLSequenceControlView : public QWidget
{
    Q_OBJECT

    public:
        CPLSequenceControlView(QWidget *parent = nullptr);
        virtual ~CPLSequenceControlView();
    protected:
    private:
};

#endif // CPLSEQUENCECONTROLVIEW_H
