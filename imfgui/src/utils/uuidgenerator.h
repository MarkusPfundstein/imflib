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

std::string ULStr(const std::string &ul);

std::string UUIDStr(const std::string &uuid);

void UUIDClean(std::string &s);

#endif // UUIDGENERATOR_H
