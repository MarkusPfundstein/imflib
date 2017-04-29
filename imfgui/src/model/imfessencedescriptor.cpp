#include "imfessencedescriptor.h"
#include <iostream>

#include "../utils/uuidgenerator.h"
#include <boost/lexical_cast.hpp>

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
    pt.put("r1:ComponentMaxref", boost::lexical_cast<std::string>(componentMaxRef));
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
}

void IMFEssenceDescriptor::AudioChannelLabelSubDescriptor::Write(boost::property_tree::ptree& pt) const
{
    MCALabelSubDescriptor::Write(pt);
}
void IMFEssenceDescriptor::SoundfieldGroupLabelSubDescriptor::Write(boost::property_tree::ptree& pt) const
{
    MCALabelSubDescriptor::Write(pt);
}


