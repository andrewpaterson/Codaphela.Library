#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/TypeNames.h"
#include "StandardLib/Objects.h"
#include "TestLib/Assert.h"
#include "ObjectTestSetup.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUsageNullPointers(void)
{
	SLogConfig	sLogConfig;

	ObjectsInit();

	Ptr<CGameWorld>			pWorld;
	CPointer				pNull = Null();
	Ptr<CPlayerVehicle>		pVehicle = Null();
	Ptr<CHarrier>			pHarrier;

	sLogConfig = gcLogger.SetSilent();

	pWorld = pNull;
	pWorld->AddTickable(pNull);
	pWorld->AddTickable(pVehicle);
	pWorld->AddTickable(Null());
	pWorld->AddPlayer(Null());

	pVehicle = pHarrier;

	AssertString("This code must compile", "This code must compile");

	pWorld = pHarrier;  //This probably shouldn't compile but it does because pHarrier (Ptr<CHarrier>) extends CPointer.

	gcLogger.SetConfig(&sLogConfig);

	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUsageDefaultPointer(void)
{
	Ptr<CBaseObject>	pBase;
	Ptr<>				pDefault;
	CPointer			pObject;

	AssertString("This code must compile", "This code must compile");
	Pass();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUsage(void)
{
	BeginTests();
	MemoryInit();
	TypesInit();
	DataIOInit();

	TestUsageNullPointers();
	TestUsageDefaultPointer();

	DataIOKill();
	TypesKill();
	MemoryKill();
	TestStatistics();
}

