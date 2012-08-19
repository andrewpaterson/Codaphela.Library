#ifndef __SERIALISED_OBJECT_H__
#define __SERIALISED_OBJECT_H__
#include "BaseLib/Bool.h"
#include "CoreLib/IndexedGeneral.h"
#include "ObjectFileGeneral.h"


class CSerialisedObject
{
protected:
	int		miLength;
	char	mszType[4];

	union
	{
		struct SSerialisedName
		{
			int		miLength;
			char	msz[4];
		} name;
		OIndex	moi;
	};

public:
	BOOL	IsNamed(void);
	BOOL	IsIndexed(void);
	BOOL	IsVoid(void);

	char*	GetName(void);
	OIndex	GetIndex(void);
	int		GetLength(void);
};

#endif // __SERIALISED_OBJECT_H__
