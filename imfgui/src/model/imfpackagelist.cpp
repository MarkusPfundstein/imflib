#include "imfpackagelist.h"

#include "imfpackageitem.h"
#include "../utils/uuidgenerator.h" 

#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/exceptions.hpp>



IMFPackageList::IMFPackageList(const std::string &uuid, const std::string& filename)
    :
    IMFPackageItem(uuid, filename, IMFPackageItem::TYPE::PKL),
    _header()
{
    //ctor
}

IMFPackageList::~IMFPackageList()
{
    //dtor
}

void IMFPackageList::Write(const std::vector<std::shared_ptr<IMFPackageItem>> &assets)
{
    using boost::property_tree::ptree;
    ptree pt;

    ptree rootNode;

    rootNode.put("<xmlattr>.xmlns", "http://www.smpte-ra.org/schemas/429-8/2007/PKL");
    rootNode.put("Id", UUIDStr(this->GetUUID()));

    _header.Write(rootNode);

    ptree assetListNode;

    for (const std::shared_ptr<IMFPackageItem>& item : assets) {
        ptree assetNode;
	// Photon expects them in PRECISE ORDER
        assetNode.put("Id", UUIDStr(item->GetUUID()));
        assetNode.put("Hash", "");
        assetNode.put("Size", boost::lexical_cast<std::string>(item->GetFileSize()));
        assetNode.put("Type", item->ApplicationType());
        assetNode.put("OriginalFileName", item->GetFileName());

        assetListNode.add_child("Asset", assetNode);
    }

    rootNode.add_child("AssetList", assetListNode);
    pt.add_child("PackingList", rootNode);

    boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
    write_xml(GetPath(), pt, std::locale(), settings);
}
