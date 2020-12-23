#ifndef __THREAD_H__
#define __THREAD_H__
#include "BaseLib/Define.h"
#include "BaseLib/Killable.h"
#include "BaseLib/Constructable.h"
#include "ThreadState.h"
#include "ThreadStateNotifier.h"


class CThread : public CConstructable, public CKillable
{
private:
	EThreadState			meState;
	int						miThreadId;
	std::thread*			mpstdThread;
	BOOL					mbDelete;
	CThreadStateNotifer*	mpcNotify;

public:
			void	Init(void);
			void	Init(CThreadStateNotifer* pcNotify);
			void	Kill(void);

			void	Start(void);

			void	TryStop(void);
			BOOL	IsDone(void);
			BOOL	IsRunning(void);
			int		GetThreadId(void);

			void	Start(int iThreadId, BOOL bDelete);
protected:
	virtual void	Run(void) =0;
			void	SetThread(std::thread* pstdThread);
			void	ChangeState(EThreadState eState);
};


#endif // __THREAD_H__

