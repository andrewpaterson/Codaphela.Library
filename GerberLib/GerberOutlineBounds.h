#ifndef __GERBER_OUTLINE_BOUNDS_H__
#define __GERBER_OUTLINE_BOUNDS_H__
#include "BaseLib/Float64Vec2.h"
#include "SupportLib/VectorPath.h"
#include "SupportLib/Float64Rect.h"
#include "GerberInterpreterListener.h"


//Finds the bounding rectangle of a board outline (the edge cuts layer) in millimetres.  The bounds are those of the
//centre lines of the draws and arcs, which is where the board is cut; the stroke width is not included.
//The rectangle's msTopLeft is the minimum corner and msBottomRight the maximum; with Gerber's y up that puts the
//lowest y in msTopLeft.
class CGerberOutlineBounds : public CGerberInterpreterListener
{
protected:
	CVectorPath		mcPath;  //The centre lines.

public:
	void	Init(void);
	void	Kill(void);

	void	Draw(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd);
	void	Arc(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd, SGerberPoint* psCenter, bool bClockwise);

	bool	GetBounds(CFloat64Rect* pcBounds);

protected:
	void	ToMillimetres(CGerberGraphicsState* pcState, SGerberPoint* psPoint, SFloat64Vec2* psMillimetres);
};


#endif // __GERBER_OUTLINE_BOUNDS_H__
