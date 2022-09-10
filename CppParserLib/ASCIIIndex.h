#ifndef __ASCII_INDEX_H__
#define __ASCII_INDEX_H__
#include "ASCIITree.h"
#include "BaseLib/IndexLongString.h"


class CASCIIIndex
{
protected:
	CASCIITree			mcNameToIDIndex;
	CIndexLongString	mcIDToNameIndex;

public:
	void	Init(void);
	void	Kill(void);

	int64	Add(char* sz, char* szLastCharInclusive = NULL);

	char*	Get(int64 lliID, int* piLength = NULL);
	int64	Get(char* szText, char* szLastCharInclusive, bool bExact);

	int		NumElements(void);
};


#endif // !__ASCII_INDEX_H__

