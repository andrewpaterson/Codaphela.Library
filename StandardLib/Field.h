#ifndef __FIELD_H__
#define __FIELD_H__
#include "BaseLib/DataTypes.h"
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"


class CClass;
class CClasses;
class CField : public CConstructable, public CKillable
{
CONSTRUCTABLE(CField);
protected:
	uint32	muiOffset;
	CClass*	mpcFieldClass;
	CClass*	mpcContainingClass;

public:
	void		Init(CClass* pcFieldClass, uint32 uiOffset, CClass* pcContainingClass);
	void		Kill(void);

	CClasses*	GetClasses(void);
};


#endif // __FIELD_H__

