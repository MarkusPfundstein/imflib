#ifndef CPLITEMLIST_H
#define CPLITEMLIST_H

#include <list>
#include <memory>
#include <string>
#include <algorithm>

template <typename Type>
class CPLItemList
{
    typedef std::list<std::shared_ptr<Type>> TypeList;

    public:
        std::shared_ptr<Type> FindItemById(const std::string &uuid) const
        {
            auto it = std::find_if(_items.begin(),
                                    _items.end(),
                                    [&uuid](const std::shared_ptr<Type> &i) { return i->GetTrack()->GetUUID() == uuid; });
            if (it != _items.end()) {
                return *it;
            }

            return std::shared_ptr<Type>(nullptr);
        }

        void AppendItem(const std::shared_ptr<Type> &i)
        {
            _items.push_back(i);
        }

        void InsertItemAfter(const std::shared_ptr<Type> &item, const std::shared_ptr<Type> &other)
        {
            auto it = std::find_if(_items.begin(),
                                    _items.end(),
                                    [&other](const std::shared_ptr<Type> &v) { return v->GetTrack()->GetUUID() == other->GetTrack()->GetUUID(); });
            if (it != _items.end()) {
                it++;
            }
            _items.insert(it, item);
        }

        void InsertItemBefore(const std::shared_ptr<Type> &item, const std::shared_ptr<Type> &other)
        {
            auto it = std::find_if(_items.begin(),
                                    _items.end(),
                                    [&other](const std::shared_ptr<Type> &v) { return v->GetTrack()->GetUUID() == other->GetTrack()->GetUUID(); });
            if (it != _items.begin()) {
                it--;
            }
            _items.insert(it, item);
        }

        const TypeList& GetItems() const
        { return _items; }

    protected:
        TypeList _items;
};

#endif // CPLITEMLIST_H
