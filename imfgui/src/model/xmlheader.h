#ifndef XMLHEADER_H_INCLUDED
#define XMLHEADER_H_INCLUDED

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

static const std::string XML_HEADER_ISSUER("ContentCoders (Research and Development)");
static const std::string XML_HEADER_CREATOR("ODMedia IMF Suite dev-0.1");
static const std::string XML_HEADER_ANNOTATION("For testing purposes only");
static const std::string XML_HEADER_CONTENT_KIND("TestPackage");

struct XMLHeaderAssetMap
{
    XMLHeaderAssetMap() :
        annotation(XML_HEADER_ANNOTATION),
        issuer(XML_HEADER_ISSUER),
        creator(XML_HEADER_CREATOR),
        issueDate("2017-03-01T21:56:47.4247570Z"),
        volumeCount(1) {}

    // optional stuff
    std::string annotation;         // description of content

    // required stuff
    std::string issuer;             // entity that created the composition
    std::string creator;            // device/software that created the composition
    std::string issueDate;          // time and date when content was issued
    int volumeCount;

    virtual void Write(boost::property_tree::ptree &ptree) const
    {
        if (!annotation.empty())
            ptree.put("AnnotationText", annotation);
        if (!creator.empty())
            ptree.put("Creator", creator); ptree.put("VolumeCount", boost::lexical_cast<std::string>(volumeCount)); ptree.put("IssueDate", issueDate);
        if (!issuer.empty())
            ptree.put("Issuer", issuer);

    }

    virtual void Read(const boost::property_tree::ptree &ptree)
    {
        annotation = ptree.get<std::string>("AnnotationText", "");
        creator = ptree.get<std::string>("Creator");
        volumeCount = ptree.get<int>("VolumeCount");
        issueDate = ptree.get<std::string>("IssueDate");
        issuer = ptree.get<std::string>("Issuer");

    }

    virtual void Dump() const
    {
        std::cout << "AnnotationText: " << annotation << std::endl;
        std::cout << "Creator: " << creator << std::endl;
        std::cout << "VolumeCount: " << volumeCount << std::endl;
        std::cout << "IssueDate: " << issueDate << std::endl;
        std::cout << "Issuer: " << issuer << std::endl;

    }
};

struct XMLHeaderCompositionPlaylist
{
    XMLHeaderCompositionPlaylist() :
        annotation(XML_HEADER_ANNOTATION),
        issuer(XML_HEADER_ISSUER),
        creator(XML_HEADER_CREATOR),
        contentKind(XML_HEADER_CONTENT_KIND),
        issueDate("2017-03-01T21:56:47.4247570Z"),
        contentTitle("CONTENT_TITLE") {}

    // optional stuff
    std::string annotation;         // description of content
    std::string issuer;             // entity that created the composition
    std::string creator;            // device/software that created the composition
    std::string contentOriginator;  // ip owner of underlying content
    std::string contentKind;        // one of advertisement, feature, psa, rating,
                                    // short, teaser, test, trailer, transitional,
                                    // episode, highlights, event


    // required stuff

    std::string issueDate;          // time and date when content was issued
    std::string contentTitle;       // title of composition

    virtual void Write(boost::property_tree::ptree &ptree) const
    {
        if (!annotation.empty())
            ptree.put("Annotation", annotation);
        ptree.put("IssueDate", issueDate);
        if (!issuer.empty())
            ptree.put("Issuer", issuer);
        if (!creator.empty())
            ptree.put("Creator", creator);
        if (!contentOriginator.empty())
            ptree.put("ContentOriginator", contentOriginator);
        ptree.put("ContentTitle", contentTitle);
        if (!contentKind.empty())
            ptree.put("ContentKind", contentKind);
    }

    virtual void Read(const boost::property_tree::ptree &ptree)
    {
        annotation = ptree.get<std::string>("Annotation", "");
        issueDate = ptree.get<std::string>("IssueDate");
        issuer = ptree.get<std::string>("Issuer", "");
        creator = ptree.get<std::string>("Creator", "");
        contentOriginator = ptree.get<std::string>("ContentOriginator", "");
        contentTitle = ptree.get<std::string>("ContentTitle");
        contentKind = ptree.get<std::string>("ContentKind", "");
    }

    virtual void Dump() const
    {
        std::cout << "Annotation: " << annotation << std::endl;
        std::cout << "IssueDate: " << issueDate << std::endl;
        std::cout << "Issuer: " << issuer << std::endl;
        std::cout << "Creator: " << creator << std::endl;
        std::cout << "ContentOriginator: " << contentOriginator << std::endl;
        std::cout << "ContentTitle: " << contentTitle << std::endl;
        std::cout << "ContentKind: " << contentKind << std::endl;
    }
};

struct XMLHeaderPackageList
{
    XMLHeaderPackageList() :
        issuer(XML_HEADER_ISSUER),
        creator(XML_HEADER_CREATOR),
        issueDate("2017-03-01T21:56:47.4247570Z")
        {}

    // required stuff
    std::string issuer;             // entity that created the composition
    std::string creator;            // device/software that created the composition
    std::string issueDate;          // time and date when content was issued

    virtual void Write(boost::property_tree::ptree &ptree) const
    {
        ptree.put("IssueDate", issueDate);
        if (!issuer.empty())
            ptree.put("Issuer", issuer);
        if (!creator.empty())
            ptree.put("Creator", creator);
    }

    virtual void Read(const boost::property_tree::ptree &ptree)
    {
        creator = ptree.get<std::string>("Creator");
        issueDate = ptree.get<std::string>("IssueDate");
        issuer = ptree.get<std::string>("Issuer");

    }

    virtual void Dump() const
    {
        std::cout << "Creator: " << creator << std::endl;
        std::cout << "IssueDate: " << issueDate << std::endl;
        std::cout << "Issuer: " << issuer << std::endl;
    }
};


#endif // XMLHEADER_H_INCLUDED
