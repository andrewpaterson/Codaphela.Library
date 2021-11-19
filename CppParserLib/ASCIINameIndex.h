#ifndef __ASCII_NAME_INDEX_H__
#define __ASCII_NAME_INDEX_H__
#include "BaseLib/PrimitiveTypes.h"


struct SASCIINameIndex
{
	int64	lliID;
	char	szName[1];

	static	int Size(char* szText, char* szLastCharInclusive);
};


#endif // !__ASCII_NAME_INDEX_H__

