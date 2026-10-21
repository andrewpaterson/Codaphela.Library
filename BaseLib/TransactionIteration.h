#ifndef __TRANSACTION_ITERATION_H__
#define __TRANSACTION_ITERATION_H__
#include "Chars.h"
#include "ArrayChars.h"
#include "IndexedGeneral.h"


//The indices and names a transaction iterates.  The lists are made when an iteration starts and returned in order,
//each once.  The transaction checks each one is still visible to it as it is returned.
class CTransactionIteration
{
protected:
	CArrayOIndex	maoiIndices;
	size			miIndexPosition;
	CArrayChars		maszNames;
	size			miNamePosition;

public:
	void			Init(void);
	void			Kill(void);

	CArrayOIndex*	StartIndices(void);
	void			SortIndices(void);
	bool			NextIndex(OIndex* poi);

	CArrayChars*	StartNames(void);
	void			SortNames(void);
	char*			NextName(void);
};


bool CopyTransactionName(CChars* pszName, bool bFound, char* szName, size* puiNameLength, size uiMaxNameLength);


#endif // __TRANSACTION_ITERATION_H__

