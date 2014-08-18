#ifndef CPLVIRTUALTRACK_H
#define CPLVIRTUALTRACK_H

#include "genericitem.h"
#include <memory>
#include <vector>

class CPLSequence;

class CPLVirtualTrack : public GenericItem
{
    public:
        CPLVirtualTrack(const std::string &uuid);
        virtual ~CPLVirtualTrack();

        void AddSequence(const std::shared_ptr<CPLSequence> &sequence);

        const std::vector<std::shared_ptr<CPLSequence>>& GetSequences() const
        { return _sequences; }

    protected:
    private:

        std::vector<std::shared_ptr<CPLSequence>> _sequences;
};

#endif // CPLVIRTUALTRACK_H
