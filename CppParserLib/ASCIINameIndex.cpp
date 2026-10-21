#include "BaseLib/StringHelper.h"
#include "ASCIINameIndex.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SASCIINameIndex::Size(const char* szText, const char* szLastCharInclusive)
{
	return (int)(StrLen(szText, szLastCharInclusive) + 1 + sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SASCIINameIndex::Init(int64 lliID, const char* szText, const char* szLastCharInclusive)
{
	mlliID = lliID;
	StrCpy(mszName, szText, szLastCharInclusive);
}

