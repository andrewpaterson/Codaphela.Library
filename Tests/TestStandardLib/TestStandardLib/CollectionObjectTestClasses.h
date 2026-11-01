#ifndef __COLLECTION_OBJECT_TEST_CLASSES_H__
#define __COLLECTION_OBJECT_TEST_CLASSES_H__
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "ObjectTestClasses.h"


class CTestArrayObject : public CObject
{
CONSTRUCTABLE(CTestArrayObject)
DESTRUCTABLE(CTestArrayObject)
public:
	CArray<CTestObject>		maChildren;
	int32					mi;
	size*					mpuiNumChildrenWhenFreed;

	void				Init(int iNum);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				AllocateTest(void);
	Ptr<CTestObject>	Get(size uiIndex);
};


#endif // __COLLECTION_OBJECT_TEST_CLASSES_H__

