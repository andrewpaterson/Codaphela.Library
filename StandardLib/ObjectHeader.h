#ifndef __OBJECT_HEADER_H__
#define __OBJECT_HEADER_H__
#include "BaseLib/Chars.h"
#include "CoreLib/IndexedGeneral.h"


class CPointerHeader
{
public:
	int		mcType;
	CChars	mszObjectName;
	OIndex	moi;

	void Init(void);
	void Init(char* szName);
	void Kill(void);
};


class CObjectHeader : public CPointerHeader
{
public:
	int		miClassSize;
	CChars	mszClassName;

	void Init(void);
	void Kill(void);
};


#endif // __OBJECT_HEADER_H__

