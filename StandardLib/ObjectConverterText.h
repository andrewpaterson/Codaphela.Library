#ifndef __OBJECT_CONVERTER_TEXT_H__
#define __OBJECT_CONVERTER_TEXT_H__
#include "ObjectConverter.h"


class CObjectSource;
class CObjectConverterText : public CObjectConverter
{
CONSTRUCTABLE(CObjectConverterText);
public:
	void			Init(void);
	void			Kill(void);

	char*			GetFileExtension(void);
	bool			IsFor(CAbstractFile* pcFile);
	CObjectSource*	CreateSource(CAbstractFile* pcFile, char* szFilename);
	CBaseObject*	Convert(CObjectSource* pcSource, char* szObjectName);
};


#endif // __OBJECT_CONVERTER_TEXT_H__

