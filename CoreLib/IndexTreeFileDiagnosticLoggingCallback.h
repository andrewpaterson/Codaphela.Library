#ifndef __INDEX_TREE_FILE_KEY_DIAGNOSTIC_LOGGING_CALLBACK_H__
#define __INDEX_TREE_FILE_KEY_DIAGNOSTIC_LOGGING_CALLBACK_H__
#include "BaseLib/Chars.h"
#include "IndexTreeFileDiagnosticCallback.h"


class CIndexTreeFileDiagnosticLoggingCallback : public CIndexTreeFileDiagnosticCallback
{
protected:
	CChars	szLog;

public:
	void	Init(void);
	void	Kill(void);

	void	Put(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	void	Get(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	void	Remove(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	void	Flush(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	void	Evict(void* pvKey, int iKeySize, void* pvData, int iDataSize);

	void	Print(CChars* psz);
	void	Dump(void);

protected:
	void	Log(char* szOperation, void* pvKey, int iKeySize, void* pvData, int iDataSize);
};

#endif // !__INDEX_TREE_FILE_KEY_DIAGNOSTIC_LOGGING_CALLBACK_H__

