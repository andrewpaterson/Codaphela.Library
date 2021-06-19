#ifndef __FIELD_H__
#define __FIELD_H__
#include "BaseLib/DataTypes.h"
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"
#include "BaseLib/DatasIO.h"
#include "Unknown.h"


class CClass;
class CClasses;
class CField : public CUnknown
{
CONSTRUCTABLE(CField);
protected:
	uint32		muiOffset;
	CClass*		mpcFieldClass;
	CClass*		mpcContainingClass;

public:
			void		Init(CClass* pcFieldClass, uint32 uiOffset, CClass* pcContainingClass, char* szName);
			void		Kill(void);

			CClasses*	GetClasses(void);
			char*		GetName(void);
			CClass*		GetClass(void);
	virtual	uint32		GetSizeOf(void) =0;

	virtual	BOOL		IsArray(void);
			uint32		GetOffset(void);
	virtual	BOOL		IsPointer(void);
	virtual	BOOL		IsPrimitive(void);
	virtual	BOOL		IsUnmanaged(void);
	virtual	BOOL		IsEmbeddedObject(void);
	virtual	SDataIO*	GetDataIO(void);

protected:
	virtual	size_t		GetNameOffset(void) =0;
};


#endif // !__FIELD_H__

