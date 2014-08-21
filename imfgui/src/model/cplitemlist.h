#ifndef CPLITEMLIST_H
#define CPLITEMLIST_H

#include <list>
#include <memory>
#include <string>
#include <algorithm>

template <typename Type>
class CPLItemList
{
    typedef std::shared_ptr<Type> PtrType;
    typedef std::list<PtrType> TypeList;

    public:
        void AppendItem(const PtrType &i)
        {
            _items.push_back(i);
        }

        // inserts item item after other
        void InsertItemAfter(const PtrType &item, const PtrType &other)
        {
            auto it = std::find_if(_items.begin(),
                                    _items.end(),
                                    [&other](const PtrType &v) { return v->GetUUID() == other->GetUUID(); });
            if (it != _items.end()) {
                it++;
            }
            _items.insert(it, item);
        }

        // inserts item item before other item
        void InsertItemBefore(const PtrType &item, const PtrType &other)
        {
            auto it = std::find_if(_items.begin(),
                                    _items.end(),
                                    [&other](const PtrType &v) { return v->GetUUID() == other->GetUUID(); });
            _items.insert(it, item);
        }

        PtrType FindItem(const std::string &id) const
        {
            auto it = std::find_if(_items.begin(),
                                    _items.end(),
                                    [&id](const PtrType &v) { return v->GetUUID() == id; });
            if (it == _items.end()) {
                return PtrType(nullptr);
            }
            return *it;
        }

        const TypeList& GetItems() const
        {
            return _items;
        }

        // deletes item if its in the list. otherwise doesn't do anything
        void DeleteItem(const PtrType &item)
        {
            _items.remove_if([&item](const PtrType& s) { return s->GetUUID() == item->GetUUID(); });
        }

        // checks if list is empty
        bool IsEmpty() const
        {
            return _items.empty();
        }

        int Size() const
        {
            return _items.size();
        }

    protected:
        TypeList _items;
};

#endif // CPLITEMLIST_H
