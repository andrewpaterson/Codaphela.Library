#ifndef __OBJECT_SOURCE_TEXT_H__
#define __OBJECT_SOURCE_TEXT_H__
#include "ObjectSingleSource.h"


class CObjectSourceText : public CObjectSingleSource
{
CONSTRUCTABLE(CObjectSource);
public:
	void		Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, char* szFileName);
	void		Kill(void);
};


#endif // __OBJECT_SOURCE_TEXT_H__

