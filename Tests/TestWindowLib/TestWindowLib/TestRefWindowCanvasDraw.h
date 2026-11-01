#ifndef __TEST_REF_WINDOW_CANVAS_DRAW_H__
#define __TEST_REF_WINDOW_CANVAS_DRAW_H__
#include "WindowLib/CanvasDraw.h"
#include "DataTestRefWindow.h"


class CTestRefWindowCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CTestRefWindowCanvasDraw);
DESTRUCTABLE(CTestRefWindowCanvasDraw);
protected:
	SDataTestRefWindow*		mpcData;

public:
	void	Init(SDataTestRefWindow* pcData);
	bool	Draw(Ptr<CCanvas> pCanvas) override;
};


#endif // __TEST_REF_WINDOW_CANVAS_DRAW_H__


