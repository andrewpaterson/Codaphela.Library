#ifndef __CLASS_H__
#define __CLASS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplateEmbedded.h"
#include "PointerField.h"
#include "EmbeddedObjectField.h"
#include "DataField.h"


#define CLASS_FLAGS_PRIMITIVE	0x01
#define CLASS_FLAGS_COMPLETE	0x02


class CPointer;
class CBaseObject;
class CPrimitiveObject;
class CClasses;
class CClass
{
protected:
	CChars									mszClassName;
	uint32									muiSize;
	EPrimitiveType							meType;
	CArrayTemplate<CPointerField>			macPointers;
	CArrayTemplate<CEmbeddedObjectField>	macEmbeddedObjects;
	CArrayTemplate<CDataField>				macDatas;
	uint16									muiFlags;
	CClasses*								mpcClassesThisIn;

public:
	void			Init(char* szClassName, uint32 uiSize, EPrimitiveType eType, CClasses* pcClasses);
	void			Kill(void);

	void			Complete(void);

	void			Pointer(CBaseObject* pcThis, CPointer* pcPointer);
	void			Primitive(CBaseObject* pcThis, CPrimitiveObject* pcPrimitive);
	void			Embedded(CBaseObject* pcThis, CBaseObject* pcObject);

	CClasses*		GetClasses(void);
	char*			GetName(void);
	EPrimitiveType	GetType(void);

	BOOL			IsComplete(void);
	void			Primitive(void);
	BOOL			IsPrimitive(void);

	CClass*			GetClass(EPrimitiveType eType);
};

#endif // __CLASS_H__

