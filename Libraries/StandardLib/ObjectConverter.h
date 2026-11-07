#ifndef __OBJECT_CONVERTER_H__
#define __OBJECT_CONVERTER_H__
#include "Unknown.h"
#include "ObjectSource.h"
#include "HollowObject.h"


class CObjects;
class CObjectConverter : public CUnknown
{
CONSTRUCTABLE(CObjectConverter)
protected:
	CObjects*	mpcObjects;		//The objects converted objects are put in.

public:
			void			Init(CObjects* pcObjects);
			void			Kill(void) override;

	virtual const char*		GetFileExtension(void) =0;
	virtual bool			IsFor(CAbstractFile* pcFile) =0;
	virtual CObjectSource*	CreateSource(CAbstractFile* pcFile, const char* szFilename) =0;
	virtual CBaseObject*	Convert(CObjectSource* pcSource, const char* szObjectName) =0;
};


#endif // __OBJECT_CONVERTER_H__

