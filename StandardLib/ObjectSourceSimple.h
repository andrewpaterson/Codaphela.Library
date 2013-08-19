#ifndef __OBJECT_SOURCE_SIMPLE_H__
#define __OBJECT_SOURCE_SIMPLE_H__
#include "ObjectSingleSource.h"


class CObjectReader;
class CObjectReaderSimple;
class CObjectSourceSimple : public CObjectSingleSource
{
BASE_FUNCTIONS(CObjectSourceSimple);
protected:
	CObjectReaderSimple*	mpcReader;

public:
	void			Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, char* szFileName);
	void			Kill(void);

	BOOL			IsSimple(void);
	BOOL			IsNative(void);
	BOOL			Contains(char* szFullName);
	CBaseObject*	Convert(char* szFullName);
	CObjectReader*	GetReader(void);
};


#endif // __OBJECT_SOURCE_SIMPLE_H__

