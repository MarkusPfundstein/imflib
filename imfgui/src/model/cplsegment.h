#ifndef CPLSEGMENT_H
#define CPLSEGMENT_H

#include "genericitem.h"
#include "cplitemlist.h"

#include <memory>
#include <boost/property_tree/ptree.hpp>

class CPLSequence;

class CPLSegment : public GenericItem, public CPLItemList<CPLSequence>
{
    public:
        CPLSegment(const std::string &uuid);
        virtual ~CPLSegment();

        void Write(boost::property_tree::ptree &pt) const;

        int GetDuration() const;
};

#endif // CPLSEGMENT_H
