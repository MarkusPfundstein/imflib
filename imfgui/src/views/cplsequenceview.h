#ifndef CPLSEQUENCEVIEW_H
#define CPLSEQUENCEVIEW_H

#include <QGraphicsView>
#include <memory>
#include <map>

class CPLSegment;
class CPLSequence;
class CPLVirtualTrack;
class CPLResource;
class CPLResourceRect;
class IMFCompositionPlaylist;

class CPLSequenceView : public QGraphicsView
{
    Q_OBJECT

    public:
        CPLSequenceView(QWidget *parent = nullptr);
        virtual ~CPLSequenceView();

        QSize minimumSizeHint() const;
        QSize sizeHint() const;
        void paintScence();

    public slots:
        // Composition playlist changed
        void CompositionPlaylistChanged(const std::shared_ptr<IMFCompositionPlaylist> &newPlaylist);

    protected:
    private:

        // renders a segment
        void RenderSegment(const CPLSegment& segment, int segIdx, int startX, int heightPerTrack, int *sequenceLength);

        // renders a sequence
        void RenderSequence(const CPLSequence& sequence, int seqIdx, int startX, int trackIdx, int heightPerTrack, int *sequenceLength);

        // renders a resource
        void RenderResource(const CPLResource& resource, int seqIdx, int startX, int trackIdx, int heightPerTrack, int *resourceLength);

        // composition playlist to render
        std::shared_ptr<IMFCompositionPlaylist> _compositionPlaylist;

        // duration of playlist
        int _playlistDuration;

        // mapping of virtual track id to render index (0, 1, 2, ..)
        std::map<std::string, int> _virtualTrackMap;
};

#endif // CPLSEQUENCEVIEW_H
