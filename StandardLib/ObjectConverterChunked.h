#ifndef __OBJECT_CONVERTER_CHUNKED_H__
#define __OBJECT_CONVERTER_CHUNKED_H__
#include "ObjectConverter.h"


class CObjectSource;
class CObjectConverterChunked : public CObjectConverter
{
BASE_FUNCTIONS(CObjectConverterChunked);
public:
	void			Init(void);
	void			Kill(void);

	char*			GetFileExtension(void);
	BOOL			IsFor(CAbstractFile* pcFile);
	CObjectSource*	CreateSource(CAbstractFile* pcFile, char* szFileName);
	CPointerObject	Convert(CAbstractFile* pcFile, char* szFileName);
};


#endif // __OBJECT_CONVERTER_CHUNKED_H__

