#ifndef __OBJECT_CONVERTER_H__
#define __OBJECT_CONVERTER_H__
#include "Unknown.h"
#include "ObjectSource.h"
#include "HollowObject.h"


class CObjectConverter : public CUnknown
{
CONSTRUCTABLE(CObjectConverter)
public:
			void			Kill(void);

	virtual const char*			GetFileExtension(void) =0;
	virtual bool			IsFor(CAbstractFile* pcFile) =0;
	virtual CObjectSource*	CreateSource(CAbstractFile* pcFile, const char* szFilename) =0;
	virtual CBaseObject*	Convert(CObjectSource* pcSource, const char* szObjectName) =0;
};


#endif // __OBJECT_CONVERTER_H__

