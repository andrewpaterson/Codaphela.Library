#ifndef __OBJECT_CONVERTER_NATIVE_H__
#define __OBJECT_CONVERTER_NATIVE_H__
#include "ObjectConverter.h"
#include "ObjectSourceChunked.h"
#include "ObjectSourceSimple.h"
#include "ExternalObjectReader.h"
#include "IndexNewOld.h"


class CIndexGenerator;
class CObjectSource;
class CObjectConverterNative : public CObjectConverter
{
CONSTRUCTABLE(CObjectConverterNative);
protected:
	CArrayIndexNewOld			mcIndexRemap;  //For testing.

public:
	void			Init(void);
	void			Kill(void);

	char*			GetFileExtension(void);
	BOOL			IsFor(CAbstractFile* pcFile);
	CObjectSource*	CreateSource(CAbstractFile* pcFile, char* szFilename);
	CBaseObject*	Convert(CObjectSource* pcSource, char* szObjectName);
	BOOL			IsNative(void);

	OIndex			TestGetNewIndexFromOld(OIndex oiNew);
};


#endif // !__OBJECT_CONVERTER_NATIVE_H__

