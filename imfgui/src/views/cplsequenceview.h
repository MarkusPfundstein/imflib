#ifndef CPLSEQUENCEVIEW_H
#define CPLSEQUENCEVIEW_H

#include <QGraphicsView>
#include <memory>
#include <map>

class CPLSegment;
class CPLSequence;
class IMFTrack;
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

        // shows a right click menu when the user makes a right click on a resource
        void ShowRightClickMenuOnResource(QPoint pos, CPLResourceRect& resourceRect);

    protected:
    private:

        // user requests to insert a track after or before a resource
        void InsertResourceAction(const CPLResourceRect& resourceRect, bool append);

        // user requests to insert a track on a new segment after or before a segment
        void InsertSegmentAction(CPLResourceRect *resourceRect, bool append);

        // deletes a resource. and if parents are empty also parents
        void DeleteResourceAction(CPLResourceRect& resourceRect);

        // appends a segment at the end of the scene
        void AppendSegment(const std::shared_ptr<CPLSegment> &segment);

        // adds a sequence to a CPLSegmentRect at a certain index offset
        void AddSequence(CPLSegmentRect* segmentRect, const std::shared_ptr<CPLSequence>& sequence);

        // appends a resource at the end of the sequence
        CPLResourceRect *AppendResource(CPLSequenceRect *sequenceRect, const std::shared_ptr<CPLResource> &resource);

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
