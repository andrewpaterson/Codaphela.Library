#ifndef __POINTER_CONTAINER_H__
#define __POINTER_CONTAINER_H__
#include "Object.h"
#include "Pointer.h"


class CPointerContainer : public CObject
{
BASE_FUNCTIONS(CPointerContainer);
public:
	Ptr<>	mp;

	void					Class(void);
	Ptr<CPointerContainer>	Init(CPointer pPointer);
	void					KillData(void);

	BOOL					Save(CObjectSerialiser* pcFile);
	BOOL					Load(CObjectDeserialiser* pcFile);
};


#endif // __POINTER_CONTAINER_H__

