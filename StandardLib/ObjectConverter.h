#ifndef __OBJECT_CONVERTER_H__
#define __OBJECT_CONVERTER_H__
#include "Unknown.h"


class CObjectConverter : public CUnknown
{
public:
	void					Init(void);
	void					Kill(void);

	virtual char*			GetFileExtension(void) =0;
	virtual BOOL			IsFor(CAbstractFile* pcFile);

	virtual CObjectSource*	CreateSource(CAbstractFile* pcFile);
};


#endif // __OBJECT_CONVERTER_H__

