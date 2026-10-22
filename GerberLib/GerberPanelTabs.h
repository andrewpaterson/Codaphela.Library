#ifndef __GERBER_PANEL_TABS_H__
#define __GERBER_PANEL_TABS_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Float64Vec2.h"
#include "GerberRasteriser.h"
#include "GerberPanelOutlines.h"


//A bridge of board material across the router gap between two facing straight edges.  If mbAcrossX the edges are
//vertical, board A's right edge at x = mfEdgeA and board B's left edge at x = mfEdgeB, and the tab runs from y =
//mfStart to mfEnd.  Otherwise the edges are horizontal, A's top and B's bottom, and the tab runs along x.
struct SGerberPanelTab
{
	size	uiBoardA;
	size	uiBoardB;
	bool	bAcrossX;
	float64	fEdgeA;
	float64	fEdgeB;
	float64	fStart;
	float64	fEnd;
};


typedef CArrayTemplate<SGerberPanelTab>	CArrayGerberPanelTab;


//Joins neighbouring boards with tabs, and makes mouse bites: a row of holes along each board's edge where its tab
//snaps off.  The holes sit in the tab with their edges on the board outline (a hole offset of minus the radius); a
//positive offset would put them into the board.  Snapping leaves small stubs on the edge to file off.  Boards are neighbours where the gap between their outlines is about the panel's router gap.  Tabs only
//go where both edges are straight, so not on rounded corners or notches.  Each straight stretch both boards share
//gets one tab per 30mm (rounded up), spread evenly.  Every board has to be joined to the others; with no frame a
//loose board would fall out.
//Mouse bites must not cut copper: every board's copper is drawn on a panel raster and a tab whose holes come within
//the clearance of copper slides along its straight edge to the nearest clear place, or is dropped if there is none.
class CGerberPanelTabs
{
protected:
	CArrayGerberPanelTab	masTabs;
	float64					mfTabWidth;
	float64					mfHoleDiameter;
	float64					mfHolePitch;
	float64					mfHoleOffset;  //How far into the board the holes' centres are; negative is into the tab.
	float64					mfTabSpacing;
	float64					mfCopperClearance;
	float64					mfTolerance;
	CGerberRaster			mcCopper;
	bool					mbCopper;
	size					muiMoved;
	size					muiDropped;

public:
	void				Init(float64 fTabWidth = 5.0, float64 fHoleDiameter = 0.5, float64 fHolePitch = 0.8, float64 fHoleOffset = -0.25, float64 fTabSpacing = 30.0, float64 fCopperClearance = 0.2);
	void				Kill(void);

	bool				Calculate(CGerberPanel* pcPanel, CGerberPanelOutlines* pcOutlines);

	size				NumTabs(void);
	SGerberPanelTab*	GetTab(size uiIndex);
	void				GetMouseBites(CArrayFloat64Vec2* pasHoles);
	void				GetTabHoles(SGerberPanelTab* psTab, CArrayFloat64Vec2* pasHoles);
	float64				GetHoleDiameter(void);
	size				NumMoved(void);  //Tabs moved off copper.
	size				NumDropped(void);  //Tabs with nowhere clear of copper.

protected:
	void				FindTabs(CGerberPanelOutlines* pcOutlines, size uiBoardA, size uiBoardB, bool bAcrossX, float64 fGap);
	void				GetStraightEdges(CGerberPanelLoop* pcLoop, bool bAcrossX, float64 fEdge, CArrayFloat64Vec2* pasRanges);
	void				AddTabs(size uiBoardA, size uiBoardB, bool bAcrossX, float64 fEdgeA, float64 fEdgeB, float64 fStart, float64 fEnd);
	bool				IsClearOfCopper(SGerberPanelTab* psTab);
	bool				DrawCopper(CGerberPanel* pcPanel);
	bool				CheckJoined(CGerberPanel* pcPanel);
};


#endif // __GERBER_PANEL_TABS_H__
