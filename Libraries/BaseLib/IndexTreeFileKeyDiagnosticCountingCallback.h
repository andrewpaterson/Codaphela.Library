#ifndef __INDEX_TREE_FILE_KEY_DIAGNOSTIC_COUNTING_CALLBACK_H__
#define __INDEX_TREE_FILE_KEY_DIAGNOSTIC_COUNTING_CALLBACK_H__
#include "IndexTreeFileDiagnosticCallback.h"


class CIndexTreeFileKeyDiagnosticCountingCallback : public CIndexTreeFileDiagnosticCallback
{
protected:
	size	miCount;
	const void*	mpvKey;
	size	miKeyLength;

public:
	void	Init(const char* szKey);
	void	Init(const void* pvKey, size iKeySize);
	void	Kill(void) override;

	void	Evict(void* pvKey, size iKeySize, void* pvData, size iDataSize) override;

protected:
	bool	Matches(void* pvKey, size iKeySize);
};


#endif // __INDEX_TREE_FILE_KEY_DIAGNOSTIC_COUNTING_CALLBACK_H__

