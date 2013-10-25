#ifndef __POINTER_CONTAINER_H__
#define __POINTER_CONTAINER_H__
#include "NamedObject.h"
#include "Pointer.h"


class CPointerContainer : public CObject
{
BASE_FUNCTIONS(CPointerContainer);
public:
	Ptr<>	mp;

	void					Class(void);
	Ptr<CPointerContainer>	Init(void);
	Ptr<CPointerContainer>	Init(CPointer& pPointer);
	Ptr<CPointerContainer>	Init(CEmbeddedObject* pcObject);
	void					KillData(void);

	void					Clear();

	BOOL					Save(CObjectSerialiser* pcFile);
	BOOL					Load(CObjectDeserialiser* pcFile);
};


class CNamedPointerContainer : public CNamedObject
{
BASE_FUNCTIONS(CNamedPointerContainer);
public:
	Ptr<>	mp;

	void						Class(void);
	Ptr<CNamedPointerContainer>	Init(void);
	Ptr<CNamedPointerContainer>	Init(CPointer& pPointer);
	Ptr<CNamedPointerContainer>	Init(CEmbeddedObject* pcObject);
	void						KillData(void);

	void						Clear();

	BOOL						Save(CObjectSerialiser* pcFile);
	BOOL						Load(CObjectDeserialiser* pcFile);

};


#endif // __POINTER_CONTAINER_H__

