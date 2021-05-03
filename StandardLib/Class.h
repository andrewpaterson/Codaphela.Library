#ifndef __CLASS_H__
#define __CLASS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplateEmbedded.h"
#include "PointerField.h"
#include "EmbeddedObjectField.h"
#include "DataField.h"


class CPointer;
class CBaseObject;
class CPrimitiveObject;
class CClasses;
class CClass
{
protected:
	CChars									mszClassName;
	uint32									muiSize;
	uint32									muiType;
	CArrayTemplate<CPointerField>			macPointers;
	CArrayTemplate<CEmbeddedObjectField>	macEmbeddedObjects;
	CArrayTemplate<CDataField>				macDatas;
	uint16									muiFlags;
	CClasses*								mpcClassesThisIn;

public:
	void		Init2(char* szClassName, CClasses* mpcClasses);
	void		Kill(void);

	void		Pointer(CBaseObject* pcThis, CPointer* pcPointer);
	void		Primitive(CBaseObject* pcThis, CPrimitiveObject* pcPrimitive);
	void		Embedded(CBaseObject* pcThis, CBaseObject* pcObject);
	CClasses*	GetClasses(void);
	CClass*		GetClass(EPrimitiveType eType);
};

#endif // __CLASS_H__

