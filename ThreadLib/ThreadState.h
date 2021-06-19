#ifndef __THREAD_STATE_H__
#define __THREAD_STATE_H__


enum EThreadState
{
	TS_Waiting,
	TS_Running,
	TS_Stopping,
	TS_Stopped,
	TS_Killed
};


#endif // !__THREAD_STATE_H__

