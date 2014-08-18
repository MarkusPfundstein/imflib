#ifndef IMFCOMPOSITIONPLAYLIST_H
#define IMFCOMPOSITIONPLAYLIST_H

#include "imfpackageitem.h"
#include "imfvideotrack.h"
#include "imfaudiotrack.h"

#include <vector>
#include <memory>

#include <boost/property_tree/ptree.hpp>

class IMFCompositionPlaylist : public IMFPackageItem
{
    public:
        IMFCompositionPlaylist(const std::string &uuid, const std::string& filename);
        virtual ~IMFCompositionPlaylist();

        // writes composition playlist to disk
        void Write() const;

        // [STATIC] loads composition playlist from disk. Throws all boost::property_tree and xml_parser exceptions...
        static std::shared_ptr<IMFCompositionPlaylist> Load(const std::string &filename, boost::property_tree::ptree& pt);

        RationalNumber GetEditRate() const
        { return _editRate; }

        void SetEditRate(RationalNumber n)
        { _editRate = n; }

    protected:
    private:
        // global edit rate of playlist
        RationalNumber _editRate;
};

#endif // IMFCOMPOSITIONPLAYLIST_H
