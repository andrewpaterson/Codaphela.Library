#ifndef __OBJECT_POINTER_ARRAYS_H__
#define __OBJECT_POINTER_ARRAYS_H__
#include "BaseLib/ArrayEmbedded.h"


class CEmbeddedObject;
typedef CArrayTemplate<CEmbeddedObject*>	CArrayEmbeddedObjectPtr;


class CBaseObject;
typedef CArrayTemplate<CBaseObject*>		CArrayBaseObjectPtr;
typedef CArrayEmbedded<CBaseObject*, 32>	CArrayEmbeddedBaseObjectPtr;


class CPointer;
typedef CArrayEmbedded<CPointer*, 32>	CArrayPointerPtr;


#endif // __OBJECT_POINTER_ARRAYS_H__

