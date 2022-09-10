#ifndef __EMBEDDED_FIELD_H__
#define __EMBEDDED_FIELD_H__
#include "Field.h"


class CBaseObject;
class CEmbeddedObjectField : public CField
{
CONSTRUCTABLE(CEmbeddedObjectField);
public:
	void			Init(CClass* pcFieldClass, uint32 uiOffset, CClass* pcContainingClass, char* szName);
	void			Kill(void);

	bool			IsEmbeddedObject(void);
	uint32			GetSizeOf(void);
	size_t			GetNameOffset(void);

	CBaseObject*	GetEmbeddedObject(CBaseObject* pcFieldContainer);
};


#endif // !__EMBEDDED_FIELD_H__

