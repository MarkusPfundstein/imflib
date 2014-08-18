#ifndef CPLSEQUENCEVIEW_H
#define CPLSEQUENCEVIEW_H

#include <QWidget>
#include <memory>


class CPLSegment;
class CPLSequence;
class CPLVirtualTrack;
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

        // renders a segment
        void RenderSegment(const CPLSegment& segment, QPainter &painter, int segIdx, int startX, int heightPerTrack, int *sequenceLength);

        // renders a sequence
        void RenderSequence(const CPLSequence& sequence, QPainter &painter, int seqIdx, int startX, int trackIdx, int heightPerTrack, int *sequenceLength);

        // composition playlist to render
        std::shared_ptr<IMFCompositionPlaylist> _compositionPlaylist;

        // duration of playlist
        int _playlistDuration;
};

#endif // CPLSEQUENCEVIEW_H
