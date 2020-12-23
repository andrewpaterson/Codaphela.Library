#ifndef __STAND_ALONE_THREAD_STARTER_H__
#define __STAND_ALONE_THREAD_STARTER_H__
#include "ThreadStarter.h"


class CStandAloneThreadStarter : public CThreadStarter
{
public:
	void StartThread(CThread* pcThread);
};


extern CStandAloneThreadStarter gcStandAloneThreadStarter;


#endif // __STAND_ALONE_THREAD_STARTER_H__

