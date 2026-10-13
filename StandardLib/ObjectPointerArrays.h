#ifndef __OBJECT_POINTER_ARRAYS_H__
#define __OBJECT_POINTER_ARRAYS_H__
#include "BaseLib/ArrayTemplateEmbedded.h"
#include "BaseLib/ArrayTemplateEmbeddedPtr.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "StackPointer.h"


class CEmbeddedObject;
typedef CArrayTemplatePtr<CEmbeddedObject>				CArrayEmbeddedObjectPtr;
typedef CArrayTemplateEmbeddedPtr<CEmbeddedObject, 8>	CArrayEmbeddedObjectPtrE8;

class CBaseObject;
typedef CArrayTemplatePtr<CBaseObject>					CArrayBaseObjectPtr;
typedef CArrayTemplateEmbeddedPtr<CBaseObject, 32>		CArrayBaseObjectPtrE32;


class CPointer;
typedef CArrayTemplateEmbedded<SStackPointer, 32>		CArrayStackPointerE32;


#endif // __OBJECT_POINTER_ARRAYS_H__

