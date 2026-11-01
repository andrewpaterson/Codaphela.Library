#ifndef __TEST_SET_OBJECT_H__
#define __TEST_SET_OBJECT_H__
#include "StandardLib/SetObject.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/ChunkFileObjectWriter.h"
#include "StandardLib/ChunkFileSystemObjectReader.h"
#include "StandardLib/PointerContainer.h"
#include "StandardLib/ClassDefines.h"
#include "ObjectTestClasses.h"


class CTestEmbeddedSet : public CObject
{
CONSTRUCTABLE(CTestEmbeddedSet)
DESTRUCTABLE(CTestEmbeddedSet)
public:
	CSetObject				mcSet;
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
void CTestEmbeddedSet::Init(void)
{
	Init(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedSet::Init(STestObjectFreedNotifier* psKilledNotifier)
{
	PreInit();
	mpsFreedNotifier = psKilledNotifier;
	if (mpsFreedNotifier)
	{
		mpsFreedNotifier->bFreed = false;
	}
	mcSet.Init();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedSet::Class(void)
{
	M_Embedded(mcSet);
	U_Pointer(mpsFreedNotifier);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedSet::Free(void)
{
	if (mpsFreedNotifier)
	{
		mpsFreedNotifier->bFreed = true;
	}
}


#endif // __TEST_SET_OBJECT_H__

