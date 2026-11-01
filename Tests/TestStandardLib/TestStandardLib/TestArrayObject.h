#ifndef __TEST_ARRAY_OBJECT_H__
#define __TEST_ARRAY_OBJECT_H__
#include "StandardLib/ArrayObject.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/ChunkFileObjectWriter.h"
#include "StandardLib/ChunkFileSystemObjectReader.h"
#include "StandardLib/PointerContainer.h"
#include "StandardLib/ClassDefines.h"
#include "ObjectTestClasses.h"


class CTestEmbeddedArray : public CObject
{
CONSTRUCTABLE(CTestEmbeddedArray)
DESTRUCTABLE(CTestEmbeddedArray)
public:
	CArrayObject				mcArray;
	STestObjectFreedNotifier*	mpsFreedNotifier;

	void	Init(void);
	void	Init(STestObjectFreedNotifier* psKilledNotifier);
	void	Class(void) override;
	void	Free(void) override;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedArray::Init(void)
{
	Init(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedArray::Init(STestObjectFreedNotifier* psKilledNotifier)
{
	PreInit();
	mpsFreedNotifier = psKilledNotifier;
	if (mpsFreedNotifier)
	{
		mpsFreedNotifier->bFreed = false;
	}
	mcArray.Init();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedArray::Class(void)
{
	M_Embedded(mcArray);
	U_Pointer(mpsFreedNotifier);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedArray::Free(void)
{
	if (mpsFreedNotifier)
	{
		mpsFreedNotifier->bFreed = true;
	}
}


#endif // __TEST_ARRAY_OBJECT_H__

