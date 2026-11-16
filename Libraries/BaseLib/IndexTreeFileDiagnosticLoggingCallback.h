#ifndef __INDEX_TREE_FILE_KEY_DIAGNOSTIC_LOGGING_CALLBACK_H__
#define __INDEX_TREE_FILE_KEY_DIAGNOSTIC_LOGGING_CALLBACK_H__
#include "Chars.h"
#include "IndexTreeFileDiagnosticCallback.h"


class CIndexTreeFileDiagnosticLoggingCallback : public CIndexTreeFileDiagnosticCallback
{
protected:
	CChars	szLog;

public:
	void	Init(void);
	void	Kill(void) override;

	void	Put(void* pvKey, size iKeySize, const void* pvData, size iDataSize) override;
	void	Get(void* pvKey, size iKeySize, void* pvData, size iDataSize) override;
	void	Remove(void* pvKey, size iKeySize, void* pvData, size iDataSize) override;
	void	Flush(void* pvKey, size iKeySize, void* pvData, size iDataSize) override;
	void	Evict(void* pvKey, size iKeySize, void* pvData, size iDataSize) override;

	void	Print(CChars* psz);
	void	Dump(void);

protected:
	void	Log(const char* szOperation, void* pvKey, size iKeySize, const void* pvData, size iDataSize);
};

#endif // __INDEX_TREE_FILE_KEY_DIAGNOSTIC_LOGGING_CALLBACK_H__

