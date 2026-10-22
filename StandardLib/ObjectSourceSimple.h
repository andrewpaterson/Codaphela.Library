#ifndef __OBJECT_SOURCE_SIMPLE_H__
#define __OBJECT_SOURCE_SIMPLE_H__
#include "ObjectSingleSource.h"


class CExternalObjectReader;
class CFileObjectReader;
class CObjectSourceSimple : public CObjectSingleSource
{
CONSTRUCTABLE(CObjectSourceSimple)
protected:
	CFileObjectReader*	mpcReader;

public:
	void			Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, const char* szFilename);
	void			Kill(void);

	bool			IsSimple(void);
	bool			IsNative(void);
	bool			Contains(const char* szFullName);
	CBaseObject*	Convert(const char* szFullName);
	CExternalObjectReader*	GetReader(void);
};


#endif // __OBJECT_SOURCE_SIMPLE_H__

