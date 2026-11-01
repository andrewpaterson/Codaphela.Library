#ifndef __TICK_TEST_CANVAS_H__
#define __TICK_TEST_CANVAS_H__
#include "SupportLib/Maps.h"
#include "WindowLib/WindowTick.h"
#include "DataTestRefWindow.h"


class CTickTestCanvas : public CWindowTick
{
CONSTRUCTABLE(CTickTestCanvas);
DESTRUCTABLE(CTickTestCanvas);
protected:
	SDataTestRefWindow*		mpcData;
	Ptr<CMaps>				mpMaps;

public:
	void	Init(SDataTestRefWindow* pcData, Ptr<CMaps> pMaps, int32 iStop);
	void	Class(void) override;

	void	Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) override;
};


#endif // __TICK_TEST_CANVAS_H__

