#ifndef SHAREDPOINTERMIMEDATA_H
#define SHAREDPOINTERMIMEDATA_H

#include <QMimeData>

#include <memory>

template <typename Type>
class SharedPointerMimeData : public QMimeData
{
    public:
        SharedPointerMimeData(const std::shared_ptr<Type> &t) : QMimeData(), _storedPtr(t)
        {}

        std::shared_ptr<Type> GetStoredPointer() const
        { return _storedPtr; }

    protected:
    private:

        std::shared_ptr<Type> _storedPtr;
};

#endif // SHAREDPOINTERMIMEDATA_H
