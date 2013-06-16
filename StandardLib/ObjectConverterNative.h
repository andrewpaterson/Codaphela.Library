#ifndef __OBJECT_CONVERTER_NATIVE_H__
#define __OBJECT_CONVERTER_NATIVE_H__
#include "ObjectConverter.h"
#include "ObjectSourceChunked.h"
#include "ObjectSourceSimple.h"
#include "DependentReadObjects.h"
#include "ObjectReader.h"


class CIndexGenerator;
class CObjectSource;
class CObjectConverterNative : public CObjectConverter
{
BASE_FUNCTIONS(CObjectConverterNative);
public:
	CDependentReadObjects	mcDependentObjects;  //This is a field for testing purposes only.

	void			Init(void);
	void			Kill(void);

	char*			GetFileExtension(void);
	BOOL			IsFor(CAbstractFile* pcFile);
	CObjectSource*	CreateSource(CAbstractFile* pcFile, char* szFileName);
	CPointerObject	Convert(CObjectSource* pcSource, char* szObjectName);
	BOOL			IsNative(void);

	OIndex			TestGetNewIndexFromOld(OIndex oiNew);
};


#endif // __OBJECT_CONVERTER_NATIVE_H__

