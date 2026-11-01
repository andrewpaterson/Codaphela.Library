#ifndef __TEST_TRANSACTION_HELPER_H__
#define __TEST_TRANSACTION_HELPER_H__
#include "BaseLib/Mutex.h"
#include "BaseLib/Codabase.h"
#include "BaseLib/WriteTransaction.h"
#include "BaseLib/ReadTransaction.h"


//A value that threads in a test set and wait for.
class CTestStage
{
protected:
	CMutex	mcMutex;
	int		miStage;

public:
	void	Init(void);
	void	Kill(void);
	void	Set(int iStage);
	int		Get(void);
	void	WaitFor(int iStage);
};


CCodabase*	CreateTransactionTestCodabase(const char* szDirectory, EDurableSync eSync);
void		AssertStoredString(CCodabase* pcDatabase, OIndex oi, const char* szExpected);
void		AssertTransactionString(CWriteTransaction* pcTransaction, OIndex oi, const char* szExpected);
void		AssertReadString(CReadTransaction* pcRead, OIndex oi, const char* szExpected);


#endif // __TEST_TRANSACTION_HELPER_H__

