#ifndef __MAP_ENTRY_H__
#define __MAP_ENTRY_H__

#include "Object.h"
#include "Pointer.h"


class CMapEntry : public CObject
{
CONSTRUCTABLE(CMapEntry)
DESTRUCTABLE(CMapEntry)
protected:
	Ptr<>	mpKey;
	Ptr<>	mpValue;

public:
	Ptr<CMapEntry>	Init(void);
	Ptr<CMapEntry>	Init(CPointer& pKey, CPointer& pValue);
	Ptr<CMapEntry>	Init(CBaseObject* pcKey, CBaseObject* pcValue);
	void			Class(void) override;
	void			Free(void) override;

	void			Clear(void);
	bool			Exists(void);
	CPointer		Key(void);
	CPointer		Value(void);
	CBaseObject*	KeyObject(void);
	CBaseObject*	ValueObject(void);
};


#endif // __MAP_ENTRY_H__

