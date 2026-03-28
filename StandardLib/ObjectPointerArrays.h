#ifndef __OBJECT_POINTER_ARRAYS_H__
#define __OBJECT_POINTER_ARRAYS_H__
#include "BaseLib/ArrayTemplateEmbedded.h"
#include "BaseLib/ArrayTemplateEmbeddedPtr.h"
#include "BaseLib/ArrayTemplatePtr.h"


class CEmbeddedObject;
typedef CArrayTemplatePtr<CEmbeddedObject>			CArrayTemplateEmbeddedObjectPtr;


class CBaseObject;
typedef CArrayTemplatePtr<CBaseObject>				CArrayBlockObjectPtr;
typedef CArrayTemplateEmbeddedPtr<CBaseObject, 32>	CArrayTemplateEmbeddedBaseObjectPtr;


class CPointer;
typedef CArrayTemplateEmbedded<SStackPointer, 32>	CArrayStackPointer;


#endif // __OBJECT_POINTER_ARRAYS_H__

