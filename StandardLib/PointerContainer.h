#ifndef __POINTER_CONTAINER_H__
#define __POINTER_CONTAINER_H__
#include "Object.h"
#include "Pointer.h"


class CPointerContainer : public CObject
{
CONSTRUCTABLE(CPointerContainer);
DESTRUCTABLE(CPointerContainer);
public:
	Ptr<>	mp;

	void					Class(void);
	Ptr<CPointerContainer>	Init(void);
	Ptr<CPointerContainer>	Init(CPointer& pPointer);
	Ptr<CPointerContainer>	Init(CEmbeddedObject* pcObject);
	void					Free(void);

	void					Clear();
};


class CNamedPointerContainer : public CObject
{
CONSTRUCTABLE(CNamedPointerContainer);
DESTRUCTABLE(CNamedPointerContainer);
public:
	Ptr<>	mp;

	Ptr<CNamedPointerContainer>	Init(void);
	Ptr<CNamedPointerContainer>	Init(CPointer& pPointer);
	Ptr<CNamedPointerContainer>	Init(CEmbeddedObject* pcObject);
	void						Free(void);
	void						Class(void);

	void						Clear();
};


#endif // __POINTER_CONTAINER_H__

