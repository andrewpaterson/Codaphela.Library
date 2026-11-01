#ifndef __TICK_TEST_REF_WINDOW_H__
#define __TICK_TEST_REF_WINDOW_H__
#include "WindowLib/WindowTick.h"
#include "DataTestRefWindow.h"


class CTickTestRefWindow : public CWindowTick
{
CONSTRUCTABLE(CTickTestRefWindow);
DESTRUCTABLE(CTickTestRefWindow);
protected:
	SDataTestRefWindow* mpcData;

public:
	void	Init(SDataTestRefWindow* pcData, int32 iStop);
	void	Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) override;
};


#endif // __TICK_TEST_REF_WINDOW_H__

