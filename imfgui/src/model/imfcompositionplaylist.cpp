#include "imfcompositionplaylist.h"

#include "../utils/uuidgenerator.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/exceptions.hpp>

IMFCompositionPlaylist::IMFCompositionPlaylist(const std::string& filename)
    :
    IMFPackageItem(filename, IMFPackageItem::TYPE::CPL),
    _editRate(0, 0),
    _videoTracks(),
    _audioTracks()
{
    //ctor
}

IMFCompositionPlaylist::~IMFCompositionPlaylist()
{
    //dtor
}

void IMFCompositionPlaylist::Write() const
{
    using boost::property_tree::ptree;
    ptree pt;

    ptree rootNode;

    rootNode.put("<xmlattr>.xmlns", "http://www.smpte-ra.org/schemas/2067-3/XXXX");
    rootNode.put("<xmlattr>.xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    rootNode.put("Id", UUIDStr(_uuid));
    rootNode.put("Annotation", XML_HEADER_ANNOTATION);
    rootNode.put("Creator", XML_HEADER_CREATOR);
    rootNode.put("IssueDate", "");
    rootNode.put("Issuer", XML_HEADER_ISSUER);
    rootNode.put("ContentOriginator", XML_HEADER_ISSUER);
    rootNode.put("ContentTitle", "TestPackage");
    rootNode.put("ContentKind", "Test");
    rootNode.put("EditRate", boost::lexical_cast<std::string>(_editRate.num) + " " + boost::lexical_cast<std::string>(_editRate.denum));

    pt.add_child("CompositionPlaylist", rootNode);

    boost::property_tree::xml_writer_settings<char> settings('\t', 1);
    write_xml(GetPath(), pt, std::locale(), settings);
}

std::shared_ptr<IMFCompositionPlaylist> IMFCompositionPlaylist::Load(const std::string &path)
{
    std::shared_ptr<IMFCompositionPlaylist> playlist(new IMFCompositionPlaylist(path));

    // parse UUID etc
    // parse EditRate

    return playlist;
}


