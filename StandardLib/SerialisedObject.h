#ifndef __SERIALISED_OBJECT_H__
#define __SERIALISED_OBJECT_H__
#include "BaseLib/IndexedGeneral.h"
#include "ObjectFileGeneral.h"


//The serialised name maps onto the output of FileWriter.
//This means it's length is strlen+1.
struct SSerialisedName
{
	int		miLength;
	char	msz[4];
};


#pragma pack(push, 1)
class CSerialisedObject
{
protected:
	size				miLength;
	char				mszMagic[4];
	char				mszType[4];
	size				miIgnored;

	OIndex				moi;
	uint16				muiSize;

	SSerialisedName		msName;

public:
	bool	IsNamed(void);
	bool	IsIndexed(void);
	bool	IsVoid(void);

	char*	GetName(void);
	OIndex	GetIndex(void);
	size	GetLength(void);
	void	SetLength(size iLength);
};
#pragma pack(pop)


#endif // __SERIALISED_OBJECT_H__

