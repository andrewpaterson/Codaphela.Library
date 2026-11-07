#ifndef __BOX_FILL_CANVAS_DRAW_H__
#define __BOX_FILL_CANVAS_DRAW_H__
#include "SupportLib/ColourARGB32.h"
#include "WindowLib/CanvasDraw.h"


//Fills its canvas with one colour as CClearCanvasDraw does, but doesn't say so (see CCanvasDraw::GetSolidColour), so
//its canvas is drawn into a native canvas and copied.  SetColour changes the colour without invalidating the canvas,
//as a careless draw would.
class CBoxFillCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CBoxFillCanvasDraw);
DESTRUCTABLE(CBoxFillCanvasDraw);
protected:
	ARGB32	msColour;

public:
	void	Init(ARGB32 sColour);
	void	Class(void) override;

	bool	Draw(Ptr<CCanvas> pCanvas) override;

	void	SetColour(ARGB32 sColour);
};


#endif // __BOX_FILL_CANVAS_DRAW_H__

