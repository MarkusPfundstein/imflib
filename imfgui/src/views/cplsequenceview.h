#ifndef CPLSEQUENCEVIEW_H
#define CPLSEQUENCEVIEW_H

#include <QWidget>
#include <memory>

class IMFCompositionPlaylist;

class CPLSequenceView : public QWidget
{
    Q_OBJECT

    public:
        CPLSequenceView(QWidget *parent = nullptr);
        virtual ~CPLSequenceView();

        QSize minimumSizeHint() const;
        QSize sizeHint() const;
        void paintEvent(QPaintEvent *event);

    public slots:
        // Composition playlist changed
        void CompositionPlaylistChanged(const std::shared_ptr<IMFCompositionPlaylist> &newPlaylist);

    protected:
    private:

        // composition playlist to render
        std::shared_ptr<IMFCompositionPlaylist> _compositionPlaylist;
};

#endif // CPLSEQUENCEVIEW_H
