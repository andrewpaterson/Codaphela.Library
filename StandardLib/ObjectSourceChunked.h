#ifndef __OBJECT_SOURCE_CHUNKED_H__
#define __OBJECT_SOURCE_CHUNKED_H__
#include "ObjectMultipleSource.h"


class CObjectSourceChunked : public CObjectMultipleSource
{
BASE_FUNCTIONS(CObjectSourceChunked);
public:
	void	Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, char* szFileName);
	void	Kill(void);

	virtual CPointerObject	Convert(char* szFullName);
};


#endif // __OBJECT_SOURCE_CHUNKED_H__

