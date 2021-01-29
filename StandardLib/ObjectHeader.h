#ifndef __OBJECT_HEADER_H__
#define __OBJECT_HEADER_H__
#include "BaseLib/Chars.h"
#include "CoreLib/IndexedGeneral.h"


class CObjectIdentifier
{
public:
	int		mcType;
	CChars	mszObjectName;
	OIndex	moi;

	void Init(void);
	void Init(OIndex oi);
	void Init(char* szName);
	void Kill(void);

	BOOL IsNamed(void);
	BOOL IsIndexed(void);
};


class CObjectHeader : public CObjectIdentifier
{
public:
	CChars	mszClassName;

	void Init(void);
	void Kill(void);
};


class CPointerHeader : public CObjectIdentifier
{
public:
	unsigned short	miNumEmbedded;
	unsigned short	miEmbeddedIndex;

	void Init(void);
	void Kill(void);
};


#endif // __OBJECT_HEADER_H__

