#ifndef __GERBER_PANEL_EDGE_CUTS_H__
#define __GERBER_PANEL_EDGE_CUTS_H__
#include "BaseLib/Chars.h"
#include "GerberPanelOutlines.h"
#include "GerberPanelTabs.h"


//The panel's edge cuts: closed outlines round the material that stays, so the boards joined by their tabs.  Each tab
//joins the outlines of its two boards: A's outline runs to the tab, across it, all the way round B's outline, back
//across the tab and on round A's.  A tab between two boards already joined by another splits the joined outline in
//two instead; the second outline is the piece of router channel between the tabs.  Board cut outs stay as they are.
//The outer outlines turn counterclockwise and the channel pieces clockwise.
class CGerberPanelEdgeCuts
{
protected:
	CArrayGerberPanelLoopPtr	mapcLoops;
	float64						mfTolerance;

public:
	void				Init(void);
	void				Kill(void);

	bool				Calculate(CGerberPanelOutlines* pcOutlines, CGerberPanelTabs* pcTabs);
	bool				Write(CChars* psz, float64 fLineWidth = 0.1);

	size				NumLoops(void);
	CGerberPanelLoop*	GetLoop(size uiIndex);

protected:
	bool				ApplyTab(SGerberPanelTab* psTab);
	bool				FindEdgeSegment(bool bAcrossX, float64 fEdge, float64 fStart, float64 fEnd, size* puiLoop, size* puiSegment);
	void				PointAt(SGerberPanelSegment* psLine, bool bAcrossX, float64 fAlong, SFloat64Vec2* psPoint);
	bool				Increases(SGerberPanelSegment* psLine, bool bAcrossX);
	void				AddLine(CGerberPanelLoop* pcLoop, SFloat64Vec2* psStart, SFloat64Vec2* psEnd);
	void				AddSegments(CGerberPanelLoop* pcLoop, CGerberPanelLoop* pcSource, size uiFrom, size uiTo);
	void				WriteCoordinate(CChars* psz, char cAxis, float64 fMillimetres);
};


#endif // __GERBER_PANEL_EDGE_CUTS_H__
