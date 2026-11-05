#include "BaseLib/TypeConverter.h"
#include "BaseLib/Socket.h"
#include "TestLib/Assert.h"


void TestDebugServer(void);
void TestDebugLaunch(void);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int __cdecl main(void)
{
	int iResult;

	InitTotalStatistics();
	TypeConverterInit();
	SocketsInit();

	TestDebugServer();
	TestDebugLaunch();

	SocketsKill();
	TypeConverterKill();
	iResult = TestTotalStatistics();
	return iResult;
}
