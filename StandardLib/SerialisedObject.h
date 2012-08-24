#ifndef __SERIALISED_OBJECT_H__
#define __SERIALISED_OBJECT_H__
#include "BaseLib/Bool.h"
#include "CoreLib/IndexedGeneral.h"
#include "ObjectFileGeneral.h"


struct SSerialisedName
{
	int		miLength;
	char	msz[4];
};


class CSerialisedObject
{
protected:
	int					miLength;
	char				mszType[4];

	OIndex				moi;

	SSerialisedName		sName;

public:
	BOOL	IsNamed(void);
	BOOL	IsIndexed(void);
	BOOL	IsVoid(void);

	char*	GetName(void);
	OIndex	GetIndex(void);
	int		GetLength(void);
	void	SetLength(int iLength);
};


#endif // __SERIALISED_OBJECT_H__

