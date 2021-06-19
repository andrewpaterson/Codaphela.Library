#ifndef __OBJECT_POINTER_ARRAYS_H__
#define __OBJECT_POINTER_ARRAYS_H__
#include "BaseLib/ArrayTemplateEmbedded.h"


class CEmbeddedObject;
typedef CArrayTemplate<CEmbeddedObject*>	CArrayTemplateEmbeddedObjectPtr;


class CBaseObject;
typedef CArrayTemplate<CBaseObject*>		CArrayBlockObjectPtr;
typedef CArrayTemplateEmbedded<CBaseObject*, 32>	CArrayTemplateEmbeddedBaseObjectPtr;


class CPointer;
typedef CArrayTemplateEmbedded<CPointer*, 32>	CArrayTypedPointerPtr;


#endif // !__OBJECT_POINTER_ARRAYS_H__

