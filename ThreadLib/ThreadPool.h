#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__
#include "BaseLib/Define.h"
#include "BaseLib/Killable.h"
#include "SafeLinkedListBlock.h"
#include "VitsenityThreadPool.h"
#include "ThreadStateNotifier.h"
#include "ThreadStarter.h"
#include "Thread.h"


class CThreadPool : public CThreadStarter,  public CThreadStateNotifer
{
private:
	CVitsenityThreadPool	mcPool;
	CSafeLinkedListBlock	mcThreads;

public:
						void	Init(void);
						void	Kill(void);

	template<class M>	M*		Add(void);

						void	StartThread(CThread* pcThread);

protected:
						void	ThreadStateChanged(CThread* pcThread, EThreadState eState);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CThreadPool::Add(void)
{
	CThread* pcThread;

	pcThread = (CThread*)mcThreads.Add(sizeof(M));
	memset(pcThread, 0, sizeof(M));
	new(pcThread) M(this, this);

	return (M*)pcThread;
}


#endif // !__THREAD_POOL_H__

