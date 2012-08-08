#ifndef __NAMED_INDEXED_SORTER_SOURCE_H__
#define __NAMED_INDEXED_SORTER_SOURCE_H__
#include "BaseLib/AbstractFile.h"
#include "BaseLib/ArrayTemplate.h"


class CNamesIndexedSorterSource
{
public:
	char*		mpszCurrent;
	filePos		miPosition;
	int			miCount;

	void	Init(int iMaxStringLength, filePos iPosition);
	void	Kill(void);

	BOOL	ReadNext(CAbstractFile* pcFile, int iWidth);
};


typedef CArrayTemplate<CNamesIndexedSorterSource> CArrayNamesIndexedSorterSource;


#endif // __NAMED_INDEXED_SORTER_SOURCE_H__

