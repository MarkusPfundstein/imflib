#ifndef GENERICITEM_H
#define GENERICITEM_H

#include <string>

class GenericItem
{
    public:
        GenericItem();
        virtual ~GenericItem();

        const std::string& GetUUID() const
        { return _uuid; }

        void SetUUID(const std::string &u)
        { _uuid = u; }

    protected:
    private:
        std::string _uuid;
};

#endif // GENERICITEM_H
