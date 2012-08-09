#ifndef __NAMED_INDEXED_SORTER_SOURCE_H__
#define __NAMED_INDEXED_SORTER_SOURCE_H__
#include "BaseLib/FileBasic.h"
#include "BaseLib/ArrayTemplate.h"


class CNamedIndexedBlock;
class CNamesIndexedSorterSource
{
public:
	CNamedIndexedBlock*		mpcCurrent;
	filePos					miPosition;
	int						miCount;

	void	Init(int iMaxStringLength, filePos iPosition);
	void	Kill(void);

	BOOL	ReadNext(CFileBasic* pcFile, int iWidth);
	BOOL	IsSmallerThan(CNamesIndexedSorterSource* pcOther);
	BOOL	HasRemaining(int iBlockChunkSize);
};


typedef CArrayTemplate<CNamesIndexedSorterSource> CArrayNamesIndexedSorterSource;


#endif // __NAMED_INDEXED_SORTER_SOURCE_H__

