#ifndef IMFESSENCEDESCRIPTOR_H
#define IMFESSENCEDESCRIPTOR_H

#include "genericitem.h"

#include <memory>
#include <string>

class IMFEssenceDescriptor : public GenericItem
{
	public:
	enum TYPE {
		CDCIEssenceDescriptor,
		RGBAEssenceDescriptor,
		WaveAudioDescriptor,
		INVALID_TYPE
	};

	public:
	// TO-DO: this uuid should be used instead of _sourceEncodingUUID in imftrack.h
	IMFEssenceDescriptor(const std::string &uuid, TYPE type)
		: GenericItem(uuid), _type(type) 
	{ }

	IMFEssenceDescriptor(const std::string &uuid)
		: IMFEssenceDescriptor(uuid, TYPE::INVALID_TYPE)
	{ }

	virtual ~IMFEssenceDescriptor()
	{ }

	void SetType(TYPE t)
 	{ _type = t; }

	TYPE GetType() const 
	{ return _type; }	

	std::string GetTypeAsString() const 
	{ 
		switch(_type) {
			case CDCIEssenceDescriptor: return "CDCIDescriptor";
			case WaveAudioDescriptor: return "WAVEPCMDescriptor";
			default: return "UNKNOWNESSENCETYPE";
		}
	}

	private:
	TYPE _type;
	
};

#endif
