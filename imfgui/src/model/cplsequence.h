#ifndef CPLSEQUENCE_H
#define CPLSEQUENCE_H

#include "genericitem.h"

#include <memory>
#include <list>
#include "cplitemlist.h"
#include <boost/property_tree/ptree.hpp>

class CPLResource;

class CPLSequence : public GenericItem, public CPLItemList<CPLResource>
{
    public:
        CPLSequence(const std::string &uuid);
        virtual ~CPLSequence();

        void Write(boost::property_tree::ptree &ptree) const;

        void SetVirtualTrackId(const std::string &v)
        { _virtualTrackId = v; }

        const std::string& GetVirtualTrackId() const
        { return _virtualTrackId; }

        int GetDuration() const;

    protected:
    private:
        std::string _virtualTrackId;
};

#endif // CPLSEQUENCE_H
