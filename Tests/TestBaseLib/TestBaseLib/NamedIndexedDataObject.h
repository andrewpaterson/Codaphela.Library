#ifndef __TEST_NAMED_INDEXED_DATA_OBJECT_H__
#define __TEST_NAMED_INDEXED_DATA_OBJECT_H__
#include "BaseLib/PrimitiveTypes.h"


class CTestNamedIndexedDataObject
{
public:
	char			mszString[256];
	int64			miNumberX;
	int64			miNumberY;

	void	Init(void);
	void	Init(const char* szString, int64 iNumberX, int64 iNumberY);

	size	Size(void);
};


#endif // __TEST_NAMED_INDEXED_DATA_OBJECT_H__

