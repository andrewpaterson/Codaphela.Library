#ifndef __THREAD_H__
#define __THREAD_H__
#include "BaseLib/Define.h"
#include "BaseLib/Killable.h"
#include "BaseLib/Constructable.h"
#include "BaseLib/ArrayTemplateEmbeddedPtr.h"
#include "ThreadState.h"
#include "ThreadStateNotifier.h"
#include "ThreadStarter.h"
#include "BaseLib/NativeThread.h"


class CThread : public CConstructable, public CKillable
{
friend class CStandAloneThreadStarter;
private:
	EThreadState										meState;
	int													miThreadId;
	CNativeThread										mcNativeThread;
	bool												mbDelete;
	CThreadStarter*										mpcStarter;
	CArrayTemplateEmbeddedPtr<CThreadStateNotifer, 2>	mapcNotifiers;

public:
					CThread(void);
					CThread(CThreadStarter* pcStarter, CThreadStateNotifer* pcNotify);

			void	Init(void);
			void	Init(CThreadStarter* pcStarter, CThreadStateNotifer* pcNotify);
			void	Kill(void);

			void	Start(void);

			void	TryStop(void);
			bool	IsDone(void);
			bool	IsRunning(void);
			int		GetThreadId(void);
			void	AddNotifier(CThreadStateNotifer* pcNotify);

			void	Start(int iThreadId, bool bDelete);
protected:
	virtual void	Run(void) =0;
			void	ChangeState(EThreadState eState);
};


#endif // __THREAD_H__

