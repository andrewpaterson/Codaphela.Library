#ifndef __KEY_EVICTED_COUNTING_CALLBACK_H__
#define __KEY_EVICTED_COUNTING_CALLBACK_H__
#include "IndexTreeFileKeyDiagnosticCallback.h"


class CKeyEvictedCoutingCallback : public CIndexTreeFileKeyDiagnosticCallback
{
protected:
	int		miCount;

public:
	void	Init(char* szKey);
	void	Init(void* pvKey, int iKeyLength);
	void	Kill(void);

	void	Evict(void* pvKey, int iKeySize, void* pvData, int iDataSize);
};

#endif // __KEY_EVICTED_COUNTING_CALLBACK_H__

