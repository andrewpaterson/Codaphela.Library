#ifndef __BORDER_CANVAS_DRAW_H__
#define __BORDER_CANVAS_DRAW_H__
#include "SupportLib/ColourARGB32.h"
#include "WindowLib/CanvasDraw.h"
#include "DataTestRefWindow.h"


class CBorderCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CBorderCanvasDraw);
DESTRUCTABLE(CBorderCanvasDraw);
protected:
	ARGB32	msColour;

public:
	void	Init(ARGB32 sColour);
	bool	Draw(Ptr<CCanvas> pCanvas) override;
};


#endif // __BORDER_CANVAS_DRAW_H__

