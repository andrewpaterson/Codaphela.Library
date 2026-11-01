#include "BaseLib/TypeConverter.h"
#include "BaseLib/MemoryDrive.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMemoryDrive(void)
{
	TypeConverterInit();
	BeginTests();
	
	CMemoryDrive	cDrive;

	char	szExpected[512];
	char	szActual[512];
	int		i;
	for (i = 0; i < 511; i++)
	{
		szExpected[i] = i % 26 + 'a';
	}
	szExpected[i] = '\0';

	cDrive.Init(512 * 16, 512);
	AssertTrue(cDrive.Write(1, szExpected));

	AssertTrue(cDrive.Read(0, szActual));
	AssertString("", szActual)

	AssertTrue(cDrive.Read(1, szActual));
	AssertString(szExpected, szActual)

	cDrive.Kill();

	TestStatistics();
	TypeConverterKill();
}

