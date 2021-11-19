#include "BaseLib/StringHelper.h"
#include "ASCIINameIndex.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SASCIINameIndex::Size(char* szText, char* szLastCharInclusive)
{
	return StrLen(szText, szLastCharInclusive) + 1 + sizeof(int64);
}

