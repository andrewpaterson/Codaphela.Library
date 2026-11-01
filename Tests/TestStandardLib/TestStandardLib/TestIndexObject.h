#ifndef __TEST_INDEX_OBJECT_H__
#define __TEST_INDEX_OBJECT_H__
#include "StandardLib/IndexObject.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/ChunkFileObjectWriter.h"
#include "StandardLib/ChunkFileSystemObjectReader.h"
#include "StandardLib/PointerContainer.h"
#include "StandardLib/ClassDefines.h"
#include "ObjectTestClasses.h"


class CTestEmbeddedIndex : public CObject
{
CONSTRUCTABLE(CTestEmbeddedIndex)
DESTRUCTABLE(CTestEmbeddedIndex)
public:
	CIndexObject				mcIndex;
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
void CTestEmbeddedIndex::Init(void)
{
	Init(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedIndex::Init(STestObjectFreedNotifier* psKilledNotifier)
{
	PreInit();
	mpsFreedNotifier = psKilledNotifier;
	if (mpsFreedNotifier)
	{
		mpsFreedNotifier->bFreed = false;
	}
	mcIndex.Init();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedIndex::Class(void)
{
	M_Embedded(mcIndex);
	U_Pointer(mpsFreedNotifier);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedIndex::Free(void)
{
	if (mpsFreedNotifier)
	{
		mpsFreedNotifier->bFreed = true;
	}
}


#endif // __TEST_INDEX_OBJECT_H__

