#ifndef __TEST_MAP_OBJECT_H__
#define __TEST_MAP_OBJECT_H__
#include "StandardLib/MapObject.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/ChunkFileObjectWriter.h"
#include "StandardLib/ChunkFileSystemObjectReader.h"
#include "StandardLib/PointerContainer.h"
#include "StandardLib/ClassDefines.h"
#include "ObjectTestClasses.h"


class CTestEmbeddedMap : public CObject
{
	CONSTRUCTABLE(CTestEmbeddedMap)
		DESTRUCTABLE(CTestEmbeddedMap)
public:
	CMapObject				mcMap;
	STestObjectFreedNotifier* mpsFreedNotifier;

	void	Init(void);
	void	Init(STestObjectFreedNotifier* psKilledNotifier);
	void	Class(void) override;
	void	Free(void) override;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedMap::Init(void)
{
	Init(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedMap::Init(STestObjectFreedNotifier* psKilledNotifier)
{
	PreInit();
	mpsFreedNotifier = psKilledNotifier;
	if (mpsFreedNotifier)
	{
		mpsFreedNotifier->bFreed = false;
	}
	mcMap.Init();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedMap::Class(void)
{
	M_Embedded(mcMap);
	U_Pointer(mpsFreedNotifier);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestEmbeddedMap::Free(void)
{
	if (mpsFreedNotifier)
	{
		mpsFreedNotifier->bFreed = true;
	}
}


#endif // __TEST_MAP_OBJECT_H__

