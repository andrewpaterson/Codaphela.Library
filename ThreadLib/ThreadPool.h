#ifndef __THREAD_H__
#define __THREAD_H__
#include "BaseLib/Define.h"
#include "BaseLib/Killable.h"
#include "VitsenityThreadPool.h"
#include "ThreadStateNotifier.h"


class CThreadPool : public CThreadStateNotifer
{
private:
	CVitsenityThreadPool	mcPool;

public:
	void	Init(void);
	void	Kill(void);

	void	ThreadStateChanged(CThread* pcThread, EThreadState eState);
};


#endif // __THREAD_H__

