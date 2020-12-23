#ifndef __THREAD_H__
#define __THREAD_H__
#include <thread>
#include "BaseLib/Define.h"
#include "BaseLib/Killable.h"
#include "BaseLib/Constructable.h"
#include "BaseLib/ArrayTemplateEmbedded.h"
#include "ThreadState.h"
#include "ThreadStateNotifier.h"
#include "ThreadStarter.h"


class CThread : public CConstructable, public CKillable
{
friend class CStandAloneThreadStarter;
private:
	EThreadState			meState;
	int						miThreadId;
	std::thread*			mpstdThread;
	BOOL					mbDelete;
	CThreadStarter*			mpcStarter;
	CArrayTemplateEmbedded<CThreadStateNotifer*, 2> mapcNotifiers;

public:
					CThread(void);
					CThread(CThreadStarter* pcStarter, CThreadStateNotifer* pcNotify);

			void	Init(void);
			void	Init(CThreadStarter* pcStarter, CThreadStateNotifer* pcNotify);
			void	Kill(void);

			void	Start(void);

			void	TryStop(void);
			BOOL	IsDone(void);
			BOOL	IsRunning(void);
			int		GetThreadId(void);
			void	AddNotifier(CThreadStateNotifer* pcNotify);

			void	Start(int iThreadId, BOOL bDelete);
protected:
	virtual void	Run(void) =0;
			void	SetThread(std::thread* pstdThread);
			void	ChangeState(EThreadState eState);
};


#endif // __THREAD_H__

