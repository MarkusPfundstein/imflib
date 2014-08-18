#ifndef CPLSEQUENCE_H
#define CPLSEQUENCE_H

#include "genericitem.h"

#include <memory>
#include <vector>

class CPLResource;

class CPLSequence : public GenericItem
{
    public:
        CPLSequence(const std::string &uuid);
        virtual ~CPLSequence();

        void SetVirtualTrackId(const std::string &v)
        { _virtualTrackId = v; }

        const std::string& GetVirtualTrackId() const
        { return _virtualTrackId; }


        void AddResource(const std::shared_ptr<CPLResource> &resource);

        const std::vector<std::shared_ptr<CPLResource>>& GetResources() const
        { return _resources; }

        int GetDuration() const;

    protected:
    private:

        std::string _virtualTrackId;
        std::vector<std::shared_ptr<CPLResource>> _resources;
};

#endif // CPLSEQUENCE_H
