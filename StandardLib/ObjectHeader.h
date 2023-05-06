#ifndef __OBJECT_HEADER_H__
#define __OBJECT_HEADER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/IndexedGeneral.h"


class CObjectIdentifier
{
public:
	int		mcType;
	CChars	mszObjectName;
	OIndex	moi;

	void	Init(void);
	void	Init(OIndex oi);
	void	Init(char* szName);
	void	Kill(void);

	bool	IsNamed(void);
	bool	IsIndexed(void);

	char*	GetName(void);
	OIndex	GetIndex();
	char*	GetType(void);
};


class CObjectHeader : public CObjectIdentifier
{
public:
	CChars	mszClassName;

	void	Init(void);
	void	Kill(void);

	char*	ClassName(void);
	char*	GetIdentifier(CChars* sz);
};


class CPointerHeader : public CObjectIdentifier
{
public:
	uint16	miNumEmbedded;
	uint16	miEmbeddedIndex;

	void Init(void);
	void Kill(void);
};


#endif // !__OBJECT_HEADER_H__

