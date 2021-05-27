#ifndef __EMBEDDED_FIELD_H__
#define __EMBEDDED_FIELD_H__
#include "Field.h"


class CEmbeddedObjectField : public CField
{
CONSTRUCTABLE(CEmbeddedObjectField);
public:
	void Init(CClass* pcFieldClass, uint32 uiOffset, CClass* pcContainingClass, char* szName);
	void Kill(void);
};


#endif // __EMBEDDED_FIELD_H__

