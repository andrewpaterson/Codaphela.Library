#ifndef __POINTER_CONTAINER_H__
#define __POINTER_CONTAINER_H__
#include "NamedObject.h"
#include "Pointer.h"


class CPointerContainer : public CObject
{
CONSTRUCTABLE(CPointerContainer);
public:
	Ptr<>	mp;

	void					Class(void);
	Ptr<CPointerContainer>	Init(void);
	Ptr<CPointerContainer>	Init(CPointer& pPointer);
	Ptr<CPointerContainer>	Init(CEmbeddedObject* pcObject);
	void					Free(void);

	void					Clear();
};


class CNamedPointerContainer : public CNamedObject
{
CONSTRUCTABLE(CNamedPointerContainer);
public:
	Ptr<>	mp;

	void						Class(void);
	Ptr<CNamedPointerContainer>	Init(void);
	Ptr<CNamedPointerContainer>	Init(CPointer& pPointer);
	Ptr<CNamedPointerContainer>	Init(CEmbeddedObject* pcObject);
	void						Free(void);

	void						Clear();
};


#endif // __POINTER_CONTAINER_H__

