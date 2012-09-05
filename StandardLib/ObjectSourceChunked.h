#ifndef __OBJECT_SOURCE_CHUNKED_H__
#define __OBJECT_SOURCE_CHUNKED_H__
#include "ObjectMultipleSource.h"


class CObjectSourceChunked : public CObjectMultipleSource
{
BASE_FUNCTIONS(CObjectSourceChunked);
public:
			void			Init(CObjectConverter* pcConverter, CAbstractFile* pcFile, char* szFileName);
			void			Kill(void);

			BOOL			Contains(char* szFullName);
	virtual CPointerObject	Convert(char* szFullName);
			BOOL			IsChunked(void);
			BOOL			IsNative(void);
};


#endif // __OBJECT_SOURCE_CHUNKED_H__

