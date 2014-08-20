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
class CPLSequenceRect;
class CPLSegmentRect;
class IMFCompositionPlaylist;
class QMouseEvent;

class CPLSequenceView : public QGraphicsView
{
    Q_OBJECT

    public:
        CPLSequenceView(QWidget *parent = nullptr);
        virtual ~CPLSequenceView();

        QSize minimumSizeHint() const;
        QSize sizeHint() const;
        void paintScence();
        void mousePressEvent(QMouseEvent *event);

    public slots:
        // Composition playlist changed
        void CompositionPlaylistChanged(const std::shared_ptr<IMFCompositionPlaylist> &newPlaylist);

    protected:
    private:

        // appends a segment at the end of the scene
        void AppendSegment(const std::shared_ptr<CPLSegment> &segment);

        // adds a sequence to a CPLSegmentRect at a certain index offset
        void AddSequence(CPLSegmentRect* segmentRect, const std::shared_ptr<CPLSequence>& sequence);

        // appends a resource at the end of the sequence
        void AppendResource(CPLSequenceRect *sequenceRect, const std::shared_ptr<CPLResource> &resource);

        // renders a segment
        void RenderSegment(const CPLSegment& segment,
                           int segIdx,
                           int startX,
                           int heightPerTrack,
                           int *sequenceLength);

        // renders a sequence
        void RenderSequence(const CPLSequence& sequence,
                            int seqIdx,
                            int startX,
                            int trackIdx,
                            int heightPerTrack,
                            int *sequenceLength);

        // renders a resource
        void RenderResource(const std::shared_ptr<CPLResource>& resource,
                            int seqIdx,
                            int startX,
                            int trackIdx,
                            int heightPerTrack,
                            int *resourceLength);

        // composition playlist to render
        std::shared_ptr<IMFCompositionPlaylist> _compositionPlaylist;

        // duration of playlist
        int _playlistDuration;

        // mapping of virtual track id to render index (0, 1, 2, ..)
        std::map<std::string, int> _virtualTrackMap;

        // icon cache for rendering in resource rects
        QImage _videoIcon;
        QImage _audioIcon;

        // height per virtual track line
        int _heightPerTrack;

        // cache of last CPLSegmentRect
        CPLSegmentRect *_lastSegmentRect;

        // current zoom factor
        float _zoomFactor;
};

#endif // CPLSEQUENCEVIEW_H
