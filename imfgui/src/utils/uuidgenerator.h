#ifndef UUIDGENERATOR_H
#define UUIDGENERATOR_H

#include <string>

class UUIDGenerator
{
    public:
        UUIDGenerator();
        virtual ~UUIDGenerator();
        UUIDGenerator(const UUIDGenerator& other) = delete;
        UUIDGenerator& operator=(const UUIDGenerator& other) = delete;

        std::string MakeUUID() const;
    protected:
    private:
};

static std::string UUIDStr(const std::string &uuid) {
    return std::string("urn:uuid:") + uuid;
}

static void UUIDClean(std::string &s) {
    s.erase(0, std::string("urn:uuid:").length());  // delete urn:uuid:
}

#endif // UUIDGENERATOR_H
