#ifndef __OBJECT_CONVERTER_NATIVE_H__
#define __OBJECT_CONVERTER_NATIVE_H__
#include "ObjectConverter.h"
#include "ObjectSourceChunked.h"
#include "ObjectSourceSimple.h"


class CIndexGenerator;
class CObjectSource;
class CObjectConverterNative : public CObjectConverter
{
BASE_FUNCTIONS(CObjectConverterNative);
public:
	CIndexGenerator*	mpcIndexGenerator;
	//CArrayIndexNewOld		mcIndexRemap;

	void			Init(CIndexGenerator* pcIndexGenerator);
	void			Kill(void);

	char*			GetFileExtension(void);
	BOOL			IsFor(CAbstractFile* pcFile);
	CObjectSource*	CreateSource(CAbstractFile* pcFile, char* szFileName);
	CPointerObject	Convert(CAbstractFile* pcFile);
	BOOL			IsNative(void);
};


#endif // __OBJECT_CONVERTER_NATIVE_H__

