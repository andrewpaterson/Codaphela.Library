#ifndef __OBJECT_SOURCE_NATIVE_H__
#define __OBJECT_SOURCE_NATIVE_H__
#include "Base.h"
#include "ObjectReader.h"


class CObjectSourceNative : public CBase
{
BASE_FUNCTIONS(CObjectSourceNative);
public:
	virtual CObjectReader*	GetReader(void) =0;
};


#endif // __OBJECT_SOURCE_NATIVE_H__

