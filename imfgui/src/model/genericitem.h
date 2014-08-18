#ifndef GENERICITEM_H
#define GENERICITEM_H

#include <string>

class GenericItem
{
    public:
        GenericItem(const std::string &uuid);
        virtual ~GenericItem();

        const std::string& GetUUID() const
        { return _uuid; }

    protected:
    private:
        std::string _uuid;
};

#endif // GENERICITEM_H
