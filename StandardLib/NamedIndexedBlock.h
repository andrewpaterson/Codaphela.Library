#ifndef __NAMED_INDEXED_BLOCK_H__
#define __NAMED_INDEXED_BLOCK_H__
#include "BaseLib/Bool.h"
#include "CoreLib/IndexedGeneral.h"


#define MAX_INDEXED_NAME_LENGTH		4087


class CNamedIndexedBlock
{
protected:
	OIndex	moi;
	char	mszName[MAX_INDEXED_NAME_LENGTH+1];

public:
	char*	Name(void);
	OIndex	Id(void);
	void	Set(char* szName, OIndex oi);
	BOOL	IsEmpty(void);
	int		CompareTo(CNamedIndexedBlock* pcOther);
	void	Zero(int iBlockWidth);
};


int CompareNamedIndexedBlock(const void* arg1, const void* arg2);


#endif // __NAMED_INDEXED_BLOCK_H__

