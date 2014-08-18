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
    _hash("")
{
    //ctor
}

CPLResource::~CPLResource()
{
    //dtor
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
    // but no idea yet to what field in MXF file
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
        throw IMFCompositionPlaylistException("Track referenced in CPL which is not defined in ASSETMAP");
    }

    std::shared_ptr<CPLResource> cplResource(new CPLResource(resourceId, *it));
    cplResource->SetEntryPoint(entryPoint);
    cplResource->SetSourceDuration(sourceDuration);
    cplResource->SetRepeatCount(repeatCount);
    cplResource->SetSourceEncoding(sourceEncodingId);
    cplResource->SetKeyId(keyId);
    cplResource->SetHash(hash);

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
