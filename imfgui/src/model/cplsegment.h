#ifndef CPLSEGMENT_H
#define CPLSEGMENT_H

#include "genericitem.h"

#include <memory>
#include <vector>

class CPLSequence;

class CPLSegment : public GenericItem
{
    public:
        CPLSegment(const std::string &uuid);
        virtual ~CPLSegment();

        void AddSequence(const std::shared_ptr<CPLSequence> &sequence);

        const std::vector<std::shared_ptr<CPLSequence>>& GetSequences() const
        { return _sequences; }

        int GetDuration() const;

    protected:
    private:

        std::vector<std::shared_ptr<CPLSequence>> _sequences;

};

#endif // CPLSEGMENT_H
