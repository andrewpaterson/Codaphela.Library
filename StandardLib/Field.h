#ifndef __FIELD_H__
#define __FIELD_H__
#include "BaseLib/DataTypes.h"
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"


class CField : public CConstructable, public CKillable
{
CONSTRUCTABLE(CField);
protected:
	uint32	muiType;
	uint32	muiOffset;

public:
	void Init(uint32 uiType, uint32 uiOffset);
	void Kill(void);
};


#endif // __FIELD_H__

