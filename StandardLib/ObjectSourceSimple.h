#ifndef __OBJECT_SOURCE_SIMPLE_H__
#define __OBJECT_SOURCE_SIMPLE_H__
#include "ObjectSingleSource.h"


class CExternalObjectReader;
class CFileObjectReader;
class CObjectSourceSimple : public CObjectSingleSource
{
CONSTRUCTABLE(CObjectSourceSimple);
protected:
	CFileObjectReader*	mpcReader;

public:
	void			Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, char* szFilename);
	void			Kill(void);

	BOOL			IsSimple(void);
	BOOL			IsNative(void);
	BOOL			Contains(char* szFullName);
	CBaseObject*	Convert(char* szFullName);
	CExternalObjectReader*	GetReader(void);
};


#endif // !__OBJECT_SOURCE_SIMPLE_H__

