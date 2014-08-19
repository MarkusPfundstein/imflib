#ifndef CPLSEQUENCEVIEW_H
#define CPLSEQUENCEVIEW_H

#include <QWidget>
#include <memory>
#include <map>

class CPLSegment;
class CPLSequence;
class CPLVirtualTrack;
class CPLResource;
class CPLResourceRect;
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

        // a resource rect got selected
        void ResourceRectGotSelected(CPLResourceRect *resourceRect);

    protected:
    private:

        // renders a segment
        void RenderSegment(const CPLSegment& segment, QPainter &painter, int segIdx, int startX, int heightPerTrack, int *sequenceLength);

        // renders a sequence
        void RenderSequence(const CPLSequence& sequence, QPainter &painter, int seqIdx, int startX, int trackIdx, int heightPerTrack, int *sequenceLength);

        // renders a resource
        void RenderResource(const CPLResource& resource, QPainter &painter, int seqIdx, int startX, int trackIdx, int heightPerTrack, int *resourceLength);

        // composition playlist to render
        std::shared_ptr<IMFCompositionPlaylist> _compositionPlaylist;

        // duration of playlist
        int _playlistDuration;

        // mapping of virtual track id to render index (0, 1, 2, ..)
        std::map<std::string, int> _virtualTrackMap;
};

#endif // CPLSEQUENCEVIEW_H
