#ifndef __INDEX_TREE_FILE_KEY_DIAGNOSTIC_COUNTING_CALLBACK_H__
#define __INDEX_TREE_FILE_KEY_DIAGNOSTIC_COUNTING_CALLBACK_H__
#include "IndexTreeFileDiagnosticCallback.h"


class CIndexTreeFileKeyDiagnosticCountingCallback : public CIndexTreeFileDiagnosticCallback
{
protected:
	size	miCount;
	void*	mpvKey;
	size	miKeyLength;

public:
	void	Init(char* szKey);
	void	Init(void* pvKey, size iKeySize);
	void	Kill(void);

	void	Evict(void* pvKey, size iKeySize, void* pvData, size iDataSize);

protected:
	bool	Matches(void* pvKey, size iKeySize);
};


#endif // __INDEX_TREE_FILE_KEY_DIAGNOSTIC_COUNTING_CALLBACK_H__

