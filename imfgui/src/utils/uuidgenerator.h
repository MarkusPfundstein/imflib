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

#endif // UUIDGENERATOR_H
