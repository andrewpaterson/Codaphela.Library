#ifndef __OBJECT_CONVERTER_H__
#define __OBJECT_CONVERTER_H__
#include "Unknown.h"
#include "ObjectSource.h"
#include "HollowObject.h"
#include "NamedHollowObject.h"


class CObjectConverter : public CUnknown
{
BASE_FUNCTIONS(CObjectConverter);
public:
			void			Kill(void);

	virtual char*			GetFileExtension(void) =0;
	virtual BOOL			IsFor(CAbstractFile* pcFile) =0;
	virtual CObjectSource*	CreateSource(CAbstractFile* pcFile, char* szFileName) =0;
	virtual CPointer	Convert(CObjectSource* pcSource, char* szObjectName) =0;
};


#endif // __OBJECT_CONVERTER_H__

