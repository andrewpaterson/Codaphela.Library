#ifndef __THREAD_STARTER_H__
#define __THREAD_STARTER_H__


class CThread;
class CThreadStarter
{
public:
	virtual void StartThread(CThread* pcThread) =0;
};


#endif // __THREAD_STARTER_H__

