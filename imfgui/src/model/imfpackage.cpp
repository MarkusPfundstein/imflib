#include "imfpackage.h"

#include "imfvideotrack.h"
#include "imfaudiotrack.h"
#include "imfcompositionplaylist.h"
#include "imfoutputprofile.h"

#include "../utils/uuidgenerator.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#define BOOST_NO_SCOPED_ENUMS
#include <boost/filesystem.hpp>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/dom/DOM.hpp>

static xercesc::DOMElement* AppendElement(xercesc::DOMDocument *document, xercesc::DOMElement *parent, const std::string &text)
{
    XERCES_CPP_NAMESPACE_USE

    XMLCh *tempStr = new XMLCh[text.size() + 1];
    XMLString::transcode(text.c_str(), tempStr, text.size());
    DOMElement *element = document->createElement(tempStr);
    parent->appendChild(element);
    delete [] tempStr;
    return element;
}

static xercesc::DOMElement* AppendElementWithText(xercesc::DOMDocument *document, xercesc::DOMElement *parent, const std::string &text, const std::string &content)
{
    XERCES_CPP_NAMESPACE_USE

    DOMElement *element = AppendElement(document, parent, text);

    XMLCh *tempStr = new XMLCh[content.size() + 1];

    XMLString::transcode(content.c_str(), tempStr, content.size());
    DOMText *textNode = document->createTextNode(tempStr);
    element->appendChild(textNode);

    delete [] tempStr;

    return element;
}

static void AddAsset(xercesc::DOMDocument *document, xercesc::DOMElement *assetList, const IMFPackageItem &item)
{
    XERCES_CPP_NAMESPACE_USE

    DOMElement *asset = AppendElement(document, assetList, "Asset");
    AppendElementWithText(document, asset, "Id", UUIDStr(item.GetUUID()));

    DOMElement *chunkList = AppendElement(document, asset, "ChunkList");
    DOMElement *chunk = AppendElement(document, chunkList, "Chunk");

    AppendElementWithText(document, chunk, "Path", item.GetFileName());
    AppendElementWithText(document, chunk, "VolumeIndex", "1");
}

static void DumpXmlToDisk(xercesc::DOMDocument *document, const std::string& filename)
{
    XERCES_CPP_NAMESPACE_USE

    XMLCh tempStr[3];

    XMLString::transcode("LS", tempStr, sizeof(tempStr));
    DOMImplementation *implementation = DOMImplementationRegistry::getDOMImplementation(tempStr);

    DOMLSSerializer *serializer = ((DOMImplementationLS*)implementation)->createLSSerializer();
    if (serializer->getDomConfig()->canSetParameter(XMLUni::fgDOMWRTDiscardDefaultContent, true)) {
        serializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTDiscardDefaultContent, true);
    }

    if (serializer->getDomConfig()->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true)) {
        serializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
    }

    DOMLSOutput *output = ((DOMImplementationLS*)implementation)->createLSOutput();

    LocalFileFormatTarget *target = new LocalFileFormatTarget(filename.c_str());

    output->setByteStream(target);
    serializer->write(document, output);

    delete target;
    output->release();
    serializer->release();
}


IMFPackage::IMFPackage()
    :
    _name(""),
    _saved(false),
    _location(""),
    _uuid(""),
    _videoTracks(),
    _audioTracks(),
    _compositionPlaylists(),
    _outputProfiles()
{
    //ctor
    std::cout << "create new package" << std::endl;
}

IMFPackage::~IMFPackage()
{
    //dtor
    std::cout << "delete package" << std::endl;
}

void IMFPackage::AddVideoTrack(const std::shared_ptr<IMFVideoTrack> &track)
{
    std::cout << "add video track: " << track->GetFileName() << std::endl;
    _videoTracks.push_back(track);
}

void IMFPackage::AddAudioTrack(const std::shared_ptr<IMFAudioTrack> &track)
{
    std::cout << "add audio track: " << track->GetFileName() << std::endl;
    _audioTracks.push_back(track);
}

bool IMFPackage::HasTrackFile(const std::string &file) const
{
    return HasVideoTrackFile(file) || HasAudioTrackFile(file);
}

bool IMFPackage::HasVideoTrackFile(const std::string &file) const
{
    auto it = std::find_if(_videoTracks.begin(), _videoTracks.end(), [&file](const std::shared_ptr<IMFVideoTrack> &vt) { return vt->GetFileName() == file; });
    return it != _videoTracks.end();
}

bool IMFPackage::HasAudioTrackFile(const std::string &file) const
{
    auto it = std::find_if(_audioTracks.begin(), _audioTracks.end(), [&file](const std::shared_ptr<IMFAudioTrack> &vt) { return vt->GetFileName() == file; });
    return it != _audioTracks.end();
}

void IMFPackage::CopyTrackFiles() const
{
    using namespace boost::filesystem;

    for (std::shared_ptr<IMFVideoTrack> asset : _videoTracks) {

        std::string newLocation = _location + "/" + _name + "/" + asset->GetFileName();

        path src(asset->GetPath());
        path target(newLocation);

        if (exists(target) == false) {
            std::cout << "Copy " << asset->GetFileName() << " to package.... can take a bit..." << std::endl;
            copy_file(src, target);
            asset->SetPath(target.string());
        }
    }

    for (std::shared_ptr<IMFAudioTrack> asset : _audioTracks) {
        std::string newLocation = _location + "/" + _name + "/" + asset->GetFileName();

        path src(asset->GetPath());
        path target(newLocation);

        if (exists(target) == false) {
            std::cout << "Copy " << asset->GetFileName() << " to package.... can take a bit..." << std::endl;
            copy_file(src, target);
            asset->SetPath(target.string());
        }
    }

    std::cout << "Copying Done" << std::endl;
}

void IMFPackage::Write() const
{
    WriteAssetMap(_location + "/" + _name + "/ASSETMAP.xml");
}

void IMFPackage::WriteAssetMap(const std::string& filename) const
{
    XERCES_CPP_NAMESPACE_USE

    XMLPlatformUtils::Initialize();
    // http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113/core.html
    DOMImplementation *domImplementation =
        DOMImplementationRegistry::getDOMImplementation(XMLString::transcode("core"));

    // pretty unsafe actually -.-
    XMLCh tempStr[256];

    XMLString::transcode("AssetMap", tempStr, sizeof(tempStr));
    DOMDocument *document = domImplementation->createDocument(0, tempStr, 0);

    DOMElement *root = document->getDocumentElement();

    XMLString::transcode("xmlns", tempStr, sizeof(tempStr));
    DOMAttr *attr = document->createAttribute(tempStr);

    XMLString::transcode("http://www.smpte-ra.org/schemas/429-9/2007/AM", tempStr, sizeof(tempStr));
    attr->setValue(tempStr);
    root->setAttributeNode(attr);

    AppendElementWithText(document, root, "Id", UUIDStr(_uuid));
    AppendElementWithText(document, root, "Creator", "ODMedia IMF Suite PRE-PRE-alpha-0.1 2014");
    AppendElementWithText(document, root, "VolumeCount", "1");
    AppendElementWithText(document, root, "IssueDate", "To-be-implemented");
    AppendElementWithText(document, root, "Issuer", "ODMedia");
    DOMElement *assetList = AppendElement(document, root, "AssetList");

    for (std::shared_ptr<IMFVideoTrack> asset : _videoTracks) {
        AddAsset(document, assetList, *asset);
    }

    for (std::shared_ptr<IMFAudioTrack> asset : _audioTracks) {
        AddAsset(document, assetList, *asset);
    }

    // Write shit to disk: TO-DO -> CAN THROW
    DumpXmlToDisk(document, filename);

    document->release();

    XMLPlatformUtils::Terminate();
}
