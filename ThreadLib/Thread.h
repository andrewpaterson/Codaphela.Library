#ifndef __THREAD_H__
#define __THREAD_H__
#include "BaseLib/Define.h"
#include "BaseLib/Killable.h"


enum EThreadState
{
	TS_Waiting,
	TS_Running,
	TS_Stopping,
	TS_Stopped,
	TS_Killed
};


class CThread 
{
private:
	EThreadState	meState;
	int				miThreadId;
	std::thread*	mpstdThread;
	BOOL			mbDelete;

public:
			void	Init(void);
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
};


#endif // __THREAD_H__

