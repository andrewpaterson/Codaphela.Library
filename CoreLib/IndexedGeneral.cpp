#include "BaseLib/StringHelper.h"
#include "IndexedGeneral.h"


char gaszIndexToStringScratchPad[64][64];
int  giIndexToStringCount = 0;

char* IndexToString(OIndex oi)
{
	int iCount = giIndexToStringCount;
	giIndexToStringCount++;
	if (giIndexToStringCount >= 64)
	{
		giIndexToStringCount = 0;
	}

	IToA(oi, gaszIndexToStringScratchPad[iCount], 10);
	return gaszIndexToStringScratchPad[iCount];
}
