#ifndef __EMBEDDED_FIELD_H__
#define __EMBEDDED_FIELD_H__
#include "Field.h"


class CEmbeddedObjectField : public CField
{
CONSTRUCTABLE(CEmbeddedObjectField);
public:
	void Init(void);
	void Kill(void);
};


#endif // __EMBEDDED_FIELD_H__

