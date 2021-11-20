#ifndef __ASCII_NAME_INDEX_H__
#define __ASCII_NAME_INDEX_H__
#include "BaseLib/PrimitiveTypes.h"


struct SASCIINameIndex
{
	int64	mlliID;
	char	mszName[1];

	static	int		Size(char* szText, char* szLastCharInclusive);

			void	Init(int64 lliID, char* szText, char* szLastCharInclusive);
};


#endif // !__ASCII_NAME_INDEX_H__

