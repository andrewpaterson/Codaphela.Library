#ifndef __OBJECT_CONVERTER_NATIVE_H__
#define __OBJECT_CONVERTER_NATIVE_H__
#include "ObjectConverter.h"
#include "ObjectSourceChunked.h"
#include "ObjectSourceSimple.h"
#include "ExternalObjectReader.h"
#include "IndexNewOld.h"


class CObjectSource;
class CObjectConverterNative : public CObjectConverter
{
CONSTRUCTABLE(CObjectConverterNative)
protected:
	CArrayIndexNewOld			mcIndexRemap;  //For testing.

public:
	void			Init(CObjects* pcObjects);
	void			Kill(void) override;

	const char*		GetFileExtension(void) override;
	bool			IsFor(CAbstractFile* pcFile) override;
	CObjectSource*	CreateSource(CAbstractFile* pcFile, const char* szFilename) override;
	CBaseObject*	Convert(CObjectSource* pcSource, const char* szObjectName) override;
	bool			IsNative(void);

	OIndex			TestGetNewIndexFromOld(OIndex oiOld);

protected:
	int32			ReadFileType(CFileBasic* pcFile);
};


#endif // __OBJECT_CONVERTER_NATIVE_H__

