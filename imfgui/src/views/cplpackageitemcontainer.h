#ifndef CPLPACKAGEITEMCONTAINER_H
#define CPLPACKAGEITEMCONTAINER_H

#include <memory>

template <typename Type>
class CPLPackageItemContainer
{
    typedef std::shared_ptr<Type> PtrType;

    public:
        CPLPackageItemContainer(const PtrType &p) :
            _item(p)
        {}

        const PtrType& GetItem() const
        { return _item; }

        void SetItem(const PtrType& ptr)
        { _item = ptr; }

    protected:
        PtrType _item;
};

#endif // CPLPACKAGEITEMCONTAINER_H
