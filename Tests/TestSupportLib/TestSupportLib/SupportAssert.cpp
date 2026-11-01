#include "TestLib/Assert.h"
#include "SupportAssert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertOPointer(CPointer pvExpected, CPointer pvActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (pvExpected.IsNotEqual(pvActual))
	{
		ToPointerString(&pvExpected, szExpected, 32);
		ToPointerString(&pvActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}

