#include "imfessencedescriptor.h"
#include <iostream>

#include "../utils/uuidgenerator.h"
#include <boost/lexical_cast.hpp>

#include <vector>
#include <map>

void WritePixelLayout(const std::string& s, boost::property_tree::ptree &pt)
{
    using namespace boost::property_tree;

    ptree layoutTree;
    if (s.empty()) {
        for (size_t i = 0; i < 8; ++i) {
            ptree comp;
            comp.put("r2:Code", "CompNull");
            comp.put("r2:ComponentSize", "0"); 
            layoutTree.add_child("r2:RGBAComponent", comp);
        }    
    }
    pt.add_child("r1:PixelLayout", layoutTree);
} 

void WritePictureComponentSizing(const std::string& s, boost::property_tree::ptree &pt)
{
    using namespace boost::property_tree;
    std::cout << "Parse PictureComponentSizing: " << s << std::endl;
    
    //example: 0000000300000003090101090101090101
    // from standard: 3,3,{11,1,1},{11,1,1},{11,1,1}
    // To-DO: verify this

    // TO-DO: rewrite this piece of shit to actually get something elegant and maintainable
    std::vector<std::string> compNames;
    compNames.push_back("XRSiz");
    compNames.push_back("YRSiz");
    compNames.push_back("Ssiz");

    ptree compSizing;
    const size_t nComps = 3;
    for (auto it = s.rbegin(); it < s.rend() - nComps * 6; it += 6) {
        // each iteration is one component
        auto it2 = it;
        auto it2e = it + 6;
        // backwards: 01 -> YRSiz, 01 ->XRSiz, 09 -> Ssiz
        int nCnt = 0;
        ptree compTree;
        do {
            char t[3];
            t[1] = *it2++;
            t[0] = *it2++;
            t[2] = '\0';
            std::string compName = compNames[nCnt++ % nComps]; 
            int c = boost::lexical_cast<int>(t);
            compTree.put("r2:" + compName, boost::lexical_cast<std::string>(c));
        } while (it2 < it2e && it2 < s.rend());
        
        compSizing.add_child("r2:J2KComponentSizing", compTree);
    }
    pt.add_child("r1:PictureComponentSizing", compSizing);
}

void WriteJ2CLayout(const std::string& s, boost::property_tree::ptree &pt)
{
    using namespace boost::property_tree;

    // TO-DO: implement that according to spec
    std::cout << "Parse J2CLayout: " << s << std::endl;
    //R(10) G(10) B(10)07f007f007ee077507750776860056005604767d367d36762

    std::map<char, std::string> compMap;
    compMap['R'] = "CompRed";
    compMap['G'] = "CompGreen";
    compMap['B'] = "CompBlue";

    const size_t nComps = 3;
    
    ptree layoutTree;
    auto ite = s.begin() + 6 * nComps;
    for (auto it = s.begin(); it < ite; it += 6) {
        std::string t(it, it+6); 

        unsigned int fir = t.find("(");
        unsigned int sec = t.find(")");

        char key = t.substr(0, 1)[0];
        std::string val = t.substr(fir + 1, sec-fir-1);

        ptree comp;
        auto mit = compMap.find(key);
        if (mit != compMap.end()) {
            comp.put("r2:Code", mit->second);
            comp.put("r2:ComponentSize", val); 
        }    
        
        layoutTree.add_child("r2:RGBAComponent", comp);
    }

    // TO-DO: check if this is ok and when this is the case
    for (size_t i = 0; i < 8 - nComps; ++i) {
        ptree comp;
        comp.put("r2:Code", "CompNull");
        comp.put("r2:ComponentSize", "0"); 
        layoutTree.add_child("r2:RGBAComponent", comp);
    }    

    pt.add_child("r1:J2CLayout", layoutTree);
}

IMFEssenceDescriptor::IMFEssenceDescriptor(const std::string &uuid, const std::shared_ptr<MXFEssenceDescriptorBase> d)
    : GenericItem(uuid),
    _essenceDescriptor(d)
{ }

IMFEssenceDescriptor::~IMFEssenceDescriptor()
{ }

void IMFEssenceDescriptor::Write(boost::property_tree::ptree &ptParent) const
{
    using namespace boost::property_tree;
    ptree pt;
    pt.put("Id", UUIDStr(GetUUID()));

    ptree essenceRoot;
    _essenceDescriptor->Write(essenceRoot);

    ptree subDescs;
    bool writeSubDescs = false;
    for (auto it = _essenceDescriptor->subDescriptors.begin(); it != _essenceDescriptor->subDescriptors.end(); ++it) {
        writeSubDescs = true;
        ptree subDesc;
        (*it)->Write(subDesc);
        subDescs.add_child("r0:" + (*it)->GetTypeAsString(), subDesc);
    }
    if (writeSubDescs) {
        essenceRoot.add_child("r1:SubDescriptors", subDescs);
    }

    pt.add_child("r0:" + _essenceDescriptor->GetTypeAsString(), essenceRoot);
    // TO-DO: Think about moving that out of this function into CompositionPlaylist
    ptParent.add_child("EssenceDescriptor", pt);
}

/* functions for EssenceDescriptors */
void IMFEssenceDescriptor::TransferFileBase::Write(boost::property_tree::ptree &pt) const
{
    pt.put("r1:InstanceID", UUIDStr(instanceId));
}

void IMFEssenceDescriptor::MXFEssenceDescriptorBase::Write(boost::property_tree::ptree &pt) const
{
    using namespace boost::property_tree;

    TransferFileBase::Write(pt);

    pt.put("r1:ContainerFormat", ULStr(containerFormat));
    pt.put("r1:SampleRate", boost::lexical_cast<std::string>(sampleRate.First()) + "/" + boost::lexical_cast<std::string>(sampleRate.Second()));
    pt.put("r1:EssenceLength", boost::lexical_cast<std::string>(essenceLength));
    pt.put("r1:LinkedTrackID", boost::lexical_cast<std::string>(linkedTrackId));
}

void IMFEssenceDescriptor::VideoEssenceDescriptor::Write(boost::property_tree::ptree &pt) const
{

    using namespace boost::property_tree;
    
    MXFEssenceDescriptorBase::Write(pt);   
    
    pt.put("r1:StoredWidth", boost::lexical_cast<std::string>(storedWidth));
    pt.put("r1:StoredHeight", boost::lexical_cast<std::string>(storedHeight));
    pt.put("r1:ImageAspectRatio", boost::lexical_cast<std::string>(imageAspectRatio.First()) + "/" + boost::lexical_cast<std::string>(imageAspectRatio.Second()));
    pt.put("r1:PictureCompression", ULStr(pictureCompression));
    // TO-DO: Figure out how to change that and how the byte that stands there must be translated
    pt.put("r1:FrameLayout", "FullFrame");
    pt.put("r1:ColorPrimaries", ULStr(colorPrimaries));
    pt.put("r1:CodingEquations", ULStr(codingEquations));
    pt.put("r1:TransferCharacteristic", ULStr(transferCharacteristic));
}

void IMFEssenceDescriptor::RGBAEssenceDescriptor::Write(boost::property_tree::ptree& pt) const
{
    VideoEssenceDescriptor::Write(pt);

    pt.put("r1:ComponentMinRef", boost::lexical_cast<std::string>(componentMinRef));
    pt.put("r1:ComponentMaxRef", boost::lexical_cast<std::string>(componentMaxRef));

    WritePixelLayout(pixelLayout, pt);
}

void IMFEssenceDescriptor::CDCIEssenceDescriptor::Write(boost::property_tree::ptree& pt) const
{
    VideoEssenceDescriptor::Write(pt);
}

void IMFEssenceDescriptor::AudioEssenceDescriptor::Write(boost::property_tree::ptree& pt) const
{
    MXFEssenceDescriptorBase::Write(pt);

    pt.put("r1:AudioSampleRate", boost::lexical_cast<std::string>(audioSampleRate.First()) + "/" + boost::lexical_cast<std::string>(audioSampleRate.Second()));
    pt.put("r1:ChannelCount", boost::lexical_cast<std::string>(channelCount));
    pt.put("r1:QuantizationBits", boost::lexical_cast<std::string>(quantizationBits));
    pt.put("r1:Locked", locked != 0 ? "True" : "False");
    pt.put("r1:SoundCompression", ULStr(soundCompression));
}

void IMFEssenceDescriptor::WaveAudioDescriptor::Write(boost::property_tree::ptree& pt) const
{
    AudioEssenceDescriptor::Write(pt);

    pt.put("r1:BlockAlign", boost::lexical_cast<std::string>(blockAlign));
    pt.put("r1:AverageBytesPerSecond", boost::lexical_cast<std::string>(averageBytesPerSecond));
    pt.put("r1:ChannelAssignment", ULStr(channelAssignment));
}

/* AUDIO SUB DESCRIPTORS */
void IMFEssenceDescriptor::MCALabelSubDescriptor::Write(boost::property_tree::ptree& pt) const
{
    TransferFileBase::Write(pt);

    pt.put("r1:MCALinkID", UUIDStr(mcaLinkId));
    pt.put("r1:MCALabelDictionaryID", ULStr(mcaLabelDictionaryId));
    pt.put("r1:MCATagSymbol", mcaTagSymbol);

    pt.put("r1:MCATagName", mcaTagName);
    pt.put("r1:RFC5646SpokenLanguage", rfc5646SpokenLanguage);

    if (!mcaAudioElementKind.empty()) {
        pt.put("r1:MCAAudioElementKind", mcaAudioElementKind);
    }
    if (!mcaAudioContentKind.empty()) {
        pt.put("r1:MCAAudioContentKind", mcaAudioContentKind);
    }
    if (!mcaTitle.empty()) {
        pt.put("r1:MCATitle", mcaTitle);
    }
    if (!mcaTitleVersion.empty()) {
        pt.put("r1:MCATitleVersion", mcaTitleVersion);
    }
}

void IMFEssenceDescriptor::AudioChannelLabelSubDescriptor::Write(boost::property_tree::ptree& pt) const
{
    MCALabelSubDescriptor::Write(pt);
    
    pt.put("r1:SoundfieldGroupLinkID", UUIDStr(soundfieldGroupLinkId));
    pt.put("r1:MCAChannelID", boost::lexical_cast<std::string>(mcaChannelId));
}

void IMFEssenceDescriptor::SoundfieldGroupLabelSubDescriptor::Write(boost::property_tree::ptree& pt) const
{
    MCALabelSubDescriptor::Write(pt);
}

void IMFEssenceDescriptor::JPEG2000SubDescriptor::Write(boost::property_tree::ptree& pt) const
{
    TransferFileBase::Write(pt);

    pt.put("r1:Xsiz", xsize);
    pt.put("r1:XOsiz", xOsize);
    pt.put("r1:XTsiz", xTsize);
    pt.put("r1:XTOsiz", xTOsize);
    pt.put("r1:Ysiz", ysize);
    pt.put("r1:YOsiz", yOsize);
    pt.put("r1:YTsiz", yTsize);
    pt.put("r1:YTOsiz", yTOsize); 
    pt.put("r1:Csiz", csize);
    pt.put("r1:Rsiz", rsize);
    pt.put("r1:CodingStyleDefault", codingStyleDefault);
    pt.put("r1:QuantizationDefault", quantizationDefault);

    WritePictureComponentSizing(pictureComponentSizing, pt);
    WriteJ2CLayout(j2cLayout, pt);
}
