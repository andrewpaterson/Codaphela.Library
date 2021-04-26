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
class CClass
{
protected:
	uint32									muiSize;
	uint32									muiType;
	CArrayTemplate<CPointerField>			macPointers;
	CArrayTemplate<CEmbeddedObjectField>	macEmbeddedObjects;
	CArrayTemplate<CDataField>				macDatas;

public:
	void Init(void);
	void Kill(void);

};

#endif // __CLASS_H__

