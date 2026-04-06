#ifndef __OBJECT_HEADER_H__
#define __OBJECT_HEADER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/IndexedGeneral.h"


#define OBJECT_IDENTIFIER_SIZE_NOT_SET	0xFFFF


class CObjectIdentifier
{
public:
	int32	meType;  //See ObjectFileGeneral.h
	CChars	mszObjectName;
	OIndex	moi;
	uint16	muiSize;  //This is the in memory allocated size (or -1).  Not the serialised size.

	void	Init(void);
	void	Init(OIndex oi, uint16 uiSize);
	void	Init(const char* szName, OIndex oi, uint16 uiSize);
	void	Kill(void);

	bool	IsNamed(void);
	bool	IsIndexed(void);
	bool	IsFat(void);

	char*	GetName(void);
	OIndex	GetIndex();
	char*	GetType(void);
	uint16	GetFatSize(void);
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


#endif // __OBJECT_HEADER_H__

