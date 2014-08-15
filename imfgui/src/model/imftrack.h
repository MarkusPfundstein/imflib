#ifndef IMFTRACK_H
#define IMFTRACK_H

#include "imfpackageitem.h"
#include "../utils/common.h"

#include <string>

class IMFTrack : public IMFPackageItem
{
    public:
        IMFTrack(const std::string& filename, TYPE type);

        virtual ~IMFTrack();

        void SetBits(int bits)
        { _bits = bits; }

        int GetBits() const
        { return _bits; }

        void SetDuration(int duration)
        { _duration = duration; }

        int GetDuration() const
        { return _duration; }

        void SetEditRate(RationalNumber editRate)
        { _editRate = editRate; }

        RationalNumber GetEditRate() const
        { return _editRate; }

    protected:
        int _bits;
        int _duration;
        RationalNumber _editRate;

};

#endif // IMFTRACK_H