#ifndef __POINTER_FIELD_H__
#define __POINTER_FIELD_H__
#include "Field.h"


class CPointerField : public CField
{
CONSTRUCTABLE(CPointerField);
public:
	void Init(void);
	void Kill(void);
};


#endif // __POINTER_FIELD_H__

