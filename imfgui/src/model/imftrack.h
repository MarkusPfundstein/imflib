#ifndef IMFTRACK_H
#define IMFTRACK_H

#include "imfpackageitem.h"
#include "imfessencedescriptor.h"
#include "../utils/common.h"

#include <memory>
#include <string>

class IMFTrack : public IMFPackageItem
{
    public:
        IMFTrack(const std::string &uuid, 
		 const std::string& filename, 
		 TYPE type,
		 std::shared_ptr<IMFEssenceDescriptor> essenceDescriptor);

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

	const IMFEssenceDescriptor& GetEssenceDescriptor() const
	{ return *_essenceDescriptor; }

    protected:
        int _bits;
        int _duration;
        RationalNumber _editRate;

	std::shared_ptr<IMFEssenceDescriptor> _essenceDescriptor;
};

#endif // IMFTRACK_H
