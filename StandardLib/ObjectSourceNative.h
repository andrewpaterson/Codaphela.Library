#ifndef __OBJECT_SOURCE_NATIVE_H__
#define __OBJECT_SOURCE_NATIVE_H__
#include "BaseLib/Bool.h"
#include "PointerObject.h"


class CObjectSourceNative
{
public:
	virtual BOOL IsChunked(void);
	virtual BOOL IsSimple(void);

	virtual CPointerObject	Convert(char* szFullName) =0;
	virtual BOOL			Contains(char* szFullName) =0;

};


#endif // __OBJECT_SOURCE_NATIVE_H__

