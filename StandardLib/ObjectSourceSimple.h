#ifndef __OBJECT_SOURCE_SIMPLE_H__
#define __OBJECT_SOURCE_SIMPLE_H__
#include "ObjectSingleSource.h"


class CObjectSourceSimple : public CObjectSingleSource
{
BASE_FUNCTIONS(CObjectSourceSimple);
public:
	void			Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, char* szFileName);
	void			Kill(void);

	BOOL			IsSimple(void);
	BOOL			IsNative(void);
	BOOL			Contains(char* szFullName);
	CPointerObject	Convert(void);
	CPointerObject	Convert(char* szFullName);
};


#endif // __OBJECT_SOURCE_SIMPLE_H__

