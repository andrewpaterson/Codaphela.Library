#ifndef __POINTER_FIELD_H__
#define __POINTER_FIELD_H__
#include "Field.h"


class CPointerField : public CField
{
CONSTRUCTABLE(CPointerField);
public:
	void Init(ptrdiff_t iOffset);
	void Kill(void);
};


#endif // __POINTER_FIELD_H__

