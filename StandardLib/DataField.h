#ifndef __DATA_FIELD_H__
#define __DATA_FIELD_H__
#include "Field.h"


class CDataField : public CField
{
CONSTRUCTABLE(CDataField);
public:
	void Init(void);
	void Kill(void);
};


#endif // __DATA_FIELD_H__

