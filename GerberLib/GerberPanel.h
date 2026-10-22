#ifndef __GERBER_PANEL_H__
#define __GERBER_PANEL_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Float64Vec2.h"
#include "BaseLib/Number.h"
#include "SupportLib/Float64Rect.h"
#include "GerberFileSets.h"


//Where one board goes on the panel.  A board point p (mm, Gerber coordinates) is placed at Rotate(p) + msOffset, where
//Rotate turns it 90 degrees counterclockwise, (x, y) -> (-y, x), if mbRotated and leaves it alone otherwise.
class CGerberPanelPlacement
{
public:
	CGerberFileSet*		mpcSet;
	bool				mbRotated;
	SFloat64Vec2		msOffset;
	int32				miOffsetX;  //msOffset in whole micrometres, exact.
	int32				miOffsetY;
	CFloat64Rect		mcBounds;  //The board's rounded outline bounds on the panel.  msTopLeft is the minimum corner.

	void	Init(CGerberFileSet* pcSet);
	void	Place(SFloat64Vec2* psPoint, SFloat64Vec2* psDest);
	void	GetOffset(CNumber* pcX, CNumber* pcY);  //Exact millimetres.
};


typedef CArrayTemplate<CGerberPanelPlacement>	CArrayGerberPanelPlacement;


//Packs boards onto one panel by their edge cuts bounding rectangles.  The gap between boards is the width of the
//router bit, so one pass separates neighbours.  The edge is the space between the boards and the panel's sides.
//The panel runs from (0, 0) to its size, in millimetres and Gerber coordinates.
//The packer works in whole micrometres with each board's outline rounded outwards, so gaps are never less than asked for.
class CGerberPanel
{
protected:
	float64						mfGap;
	float64						mfEdge;
	bool						mbAllowRotation;
	CArrayGerberPanelPlacement	macPlacements;
	SFloat64Vec2				msSize;

public:
	void					Init(float64 fGap = 1.6, float64 fEdge = 1.6, bool bAllowRotation = true);
	void					Kill(void);

	bool					Pack(CGerberFileSets* pcSets);

	SFloat64Vec2*			GetSize(void);
	float64					GetGap(void);
	float64					GetEdge(void);
	size					NumPlacements(void);
	CGerberPanelPlacement*	GetPlacement(size uiIndex);
	CGerberPanelPlacement*	GetPlacement(CGerberFileSet* pcSet);

protected:
	void					GetMicrometreBounds(CGerberFileSet* pcSet, int32* piLeft, int32* piBottom, int32* piRight, int32* piTop);
	int32					ToMicrometres(float64 fMillimetres);
};


void MillimetresToNumber(float64 fMillimetres, CNumber* pcNumber);  //Rounded to the nanometre.


#endif // __GERBER_PANEL_H__
