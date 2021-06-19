#ifndef __INDEX_TREE_FILE_KEY_DIAGNOSTIC_COUNTING_CALLBACK_H__
#define __INDEX_TREE_FILE_KEY_DIAGNOSTIC_COUNTING_CALLBACK_H__
#include "IndexTreeFileDiagnosticCallback.h"


class CIndexTreeFileKeyDiagnosticCountingCallback : public CIndexTreeFileDiagnosticCallback
{
protected:
	int		miCount;
	void*	mpvKey;
	int		miKeyLength;

public:
	void	Init(char* szKey);
	void	Init(void* pvKey, int iKeyLength);
	void	Kill(void);

	void	Evict(void* pvKey, int iKeySize, void* pvData, int iDataSize);

protected:
	BOOL	Matches(void* pvKey, int iKeySize);
};


#endif // !__INDEX_TREE_FILE_KEY_DIAGNOSTIC_COUNTING_CALLBACK_H__

