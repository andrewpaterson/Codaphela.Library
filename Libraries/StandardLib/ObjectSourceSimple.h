#ifndef __OBJECT_SOURCE_SIMPLE_H__
#define __OBJECT_SOURCE_SIMPLE_H__
#include "BaseLib/FileBasic.h"
#include "ObjectSingleSource.h"


class CExternalObjectReader;
class CFileObjectReader;
class CObjectSourceSimple : public CObjectSingleSource
{
CONSTRUCTABLE(CObjectSourceSimple)
protected:
	CFileBasic			mcFile;
	CFileObjectReader*	mpcReader;

public:
	void			Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, const char* szFilename);
	void			Kill(void) override;

	bool			IsNative(void) override;
	CBaseObject*	Convert(const char* szFullName) override;
	CExternalObjectReader*	GetReader(void) override;
};


#endif // __OBJECT_SOURCE_SIMPLE_H__

