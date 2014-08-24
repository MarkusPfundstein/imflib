#include "cplresource.h"

#include "../utils/uuidgenerator.h"

CPLResource::CPLResource(const std::string &uuid, const std::shared_ptr<IMFTrack> &track)
    :
    GenericItem(uuid),
    _track(track),
    _repeatCount(0),
    _entryPoint(0),
    _sourceDuration(0),
    _sourceEncoding(""),
    _keyId(""),
    _hash(""),
    _playlistEditRate(0, 0)
{
    //ctor
}

CPLResource::~CPLResource()
{
    //dtor
}

std::shared_ptr<CPLResource> CPLResource::StandardResource(const std::shared_ptr<IMFTrack> &track,
                                                           RationalNumber compositionPlaylistEditRate)
{
    std::shared_ptr<CPLResource> newResource(new CPLResource(UUIDGenerator().MakeUUID(), track));
    newResource->SetEntryPoint(0);
    newResource->SetSourceDuration(track->GetDuration());
    newResource->SetRepeatCount(1);
    newResource->SetPlaylistEditRate(compositionPlaylistEditRate);

    // To-DO: Fix me
    newResource->SetKeyId("");
    newResource->SetHash("");
    newResource->SetSourceEncoding(UUIDGenerator().MakeUUID());
    return newResource;
}

int CPLResource::GetNormalizedSourceDuration() const
{
    switch (_track->GetType()) {
        case IMFPackageItem::TYPE::VIDEO:
            // for video its simple. return source duration
            return _sourceDuration;
            break;

        case IMFPackageItem::TYPE::AUDIO:
            /* for audio its shitty. we have to convert the duration to video frames.
               i hacked it now by using the playlist edit rate and rounding the result.
               this is probably not really accurate and should be fixed. As I think that
               on non fractional framerates (25,30) it will cause problems */
            return roundf(_sourceDuration / _track->GetEditRate().AsDouble() * _playlistEditRate.AsDouble());
            break;
    }

    return -1;
}

void CPLResource::Write(boost::property_tree::ptree &pt) const
{
    using namespace boost::property_tree;

    ptree resource;

    std::string type = "TrackFileResourceType";

    resource.put("<xmlattr>.xsi:type", type);
    resource.put("Id", UUIDStr(GetUUID()));
    resource.put("Annotation", "");
    resource.put("EditRate", GetEditRate().AsIMFString());
    resource.put("IntrinsicDuration", GetIntrinsicDuration());
    resource.put("EntryPoint", GetEntryPoint());
    resource.put("SourceDuration", GetSourceDuration());
    resource.put("RepeatCount", GetRepeatCount());
    resource.put("SourceEncoding", UUIDStr(GetSourceEncoding()));
    resource.put("TrackFileId", UUIDStr(GetTrackFileId()));
    if (!GetKeyId().empty())
        resource.put("KeyId", GetKeyId());
    if (!GetHash().empty())
        resource.put("Hash", GetHash());

    pt.add_child("Resource", resource);

}

std::shared_ptr<CPLResource> CPLResource ::Load(const boost::property_tree::ptree &pt,
                                                const std::string &cplEditRate,
                                                const std::vector<std::shared_ptr<IMFTrack>> &tracks)
{
    std::string resourceId = pt.get<std::string>("Id");
    UUIDClean(resourceId);

    // real duration of essence in frames
    int intrinsicDuration = pt.get<int>("IntrinsicDuration");

    // entry point of essence. defaults to 0
    int entryPoint = pt.get<int>("EntryPoint", 0);

    // playing duration of essence. defaults to intrinsic - entrypoint
    int sourceDuration = pt.get<int>("SourceDuration", intrinsicDuration - entryPoint);

    // how often shall essence be played in thus sequence, defaults to 1
    int repeatCount = pt.get<int>("RepeatCount", 1);
    std::string editRateString = pt.get<std::string>("EditRate", cplEditRate);
    RationalNumber resourceEditRate = RationalNumber::FromIMFString(editRateString);

    // refers to EssenceDescriptorList in CPL
    // TO-DO check if it is already in EssenceDescriptor List . If not
    // throw IMFInvalidReferenceException
    std::string sourceEncodingId = pt.get<std::string>("SourceEncoding");
    UUIDClean(sourceEncodingId);

    // refers to tracks in ASSETMAP, thus in imfpackage
    std::string trackFileId = pt.get<std::string>("TrackFileId");
    UUIDClean(trackFileId);

    // key to decrypt essence
    std::string keyId = pt.get<std::string>("KeyId", "");

    // hash to validate essence
    std::string hash = pt.get<std::string>("Hash", "");

    // search track in tracks
    auto it = std::find_if(tracks.begin(), tracks.end(), [&trackFileId](const std::shared_ptr<IMFTrack>& t) { return t->GetUUID() == trackFileId; });
    if (it == tracks.end()) {
        throw IMFInvalidReferenceException("Track referenced in CPL which is not defined in ASSETMAP");
    }

    std::shared_ptr<CPLResource> cplResource(new CPLResource(resourceId, *it));
    cplResource->SetEntryPoint(entryPoint);
    cplResource->SetSourceDuration(sourceDuration);
    cplResource->SetRepeatCount(repeatCount);
    cplResource->SetSourceEncoding(sourceEncodingId);
    cplResource->SetKeyId(keyId);
    cplResource->SetHash(hash);
    cplResource->SetPlaylistEditRate(RationalNumber::FromIMFString(cplEditRate));

    std::cout << "\t\t\tResource ID: " << resourceId << std::endl;
    std::cout << "\t\t\tIntrinsic Duration: " << intrinsicDuration << std::endl;
    std::cout << "\t\t\tEntry Point: " << entryPoint << std::endl;
    std::cout << "\t\t\tSource Duration: " << sourceDuration << std::endl;
    std::cout << "\t\t\tRepeat Count: " << repeatCount << std::endl;
    std::cout << "\t\t\tEdit Rate: " << resourceEditRate.AsIMFString() << std::endl;
    std::cout << "\t\t\tSourceEncoding: " << sourceEncodingId << std::endl;
    std::cout << "\t\t\tTrackFileId: " << trackFileId << std::endl;
    std::cout << "\t\t\tKeyId: " << keyId << std::endl;
    std::cout << "\t\t\tHash: " << hash << std::endl;

    return cplResource;
}
