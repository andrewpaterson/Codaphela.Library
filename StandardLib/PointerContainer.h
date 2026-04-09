#ifndef __POINTER_CONTAINER_H__
#define __POINTER_CONTAINER_H__
#include "Object.h"
#include "Pointer.h"


class CPointerContainer : public CObject
{
CONSTRUCTABLE(CPointerContainer)
DESTRUCTABLE(CPointerContainer)
public:
	Ptr<>	mp;

	void					Class(void) override;
	Ptr<CPointerContainer>	Init(void);
	Ptr<CPointerContainer>	Init(CPointer& pPointer);
	Ptr<CPointerContainer>	Init(CEmbeddedObject* pcObject);
	void					Free(void) override;

	void					Clear(void);
	CEmbeddedObject*		PointerObject(void);
	CEmbeddedObject**		PointerObjectPtr(void);
};


#endif // __POINTER_CONTAINER_H__

