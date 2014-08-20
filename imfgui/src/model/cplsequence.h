#ifndef CPLSEQUENCE_H
#define CPLSEQUENCE_H

#include "genericitem.h"

#include <memory>
#include <list>

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

        std::shared_ptr<CPLResource> FindResourceByID(const std::string &uuid) const;
        void AddResource(const std::shared_ptr<CPLResource> &resource);
        void AddResourceAfterResource(const std::shared_ptr<CPLResource> &resource, const std::shared_ptr<CPLResource> &other);

        const std::list<std::shared_ptr<CPLResource>>& GetResources() const
        { return _resources; }

        int GetDuration() const;

    protected:
    private:

        std::string _virtualTrackId;
        std::list<std::shared_ptr<CPLResource>> _resources;
};

#endif // CPLSEQUENCE_H
