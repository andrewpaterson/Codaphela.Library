#ifndef __GERBER_PANEL_OUTLINES_H__
#define __GERBER_PANEL_OUTLINES_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/Float64Vec2.h"
#include "SupportLib/Float64Rect.h"
#include "GerberInterpreterListener.h"
#include "GerberPanel.h"


//A straight or circular piece of an outline in panel millimetres, y up.
struct SGerberPanelSegment
{
	bool			bArc;
	bool			bClockwise;
	SFloat64Vec2	sStart;
	SFloat64Vec2	sEnd;
	SFloat64Vec2	sCenter;  //Arcs only.
};


typedef CArrayTemplate<SGerberPanelSegment>	CArrayGerberPanelSegment;


//A closed outline; each segment starts where the previous one ends and the last ends where the first starts.
class CGerberPanelLoop
{
public:
	CArrayGerberPanelSegment	masSegments;

	void		Init(void);
	void		Kill(void);

	float64		Area(void);  //Positive counterclockwise.
	void		Reverse(void);
	void		GetBounds(CFloat64Rect* pcBounds);
	bool		IsClosed(float64 fTolerance);
};


typedef CArrayTemplatePtr<CGerberPanelLoop>	CArrayGerberPanelLoopPtr;


//Each board's edge cuts in panel coordinates, joined end to end into closed loops.  The edge cuts files draw the
//outline's pieces in any order and either direction; ends closer than the tolerance are joined exactly.  Each
//board's biggest loop is its outside and turns counterclockwise; any others are cut outs and turn clockwise.
class CGerberPanelOutlines : public CGerberInterpreterListener
{
protected:
	CArrayGerberPanelLoopPtr	mapcLoops;
	CArrayGerberPanelSegment	masPieces;  //The board being read.
	CArrayInt					maiOuterLoops;  //Each board's outside loop.
	CArrayInt					maiFirstLoops;  //Each board's first loop.
	CArrayInt					maiNumLoops;
	CGerberPanelPlacement*		mpcPlacement;  //NULL if the boards aren't placed.
	CGerberFileSet*				mpcSet;
	float64						mfTolerance;

public:
	void				Init(float64 fTolerance = 0.01);
	void				Kill(void);

	bool				Read(CGerberPanel* pcPanel);
	bool				Read(CGerberFileSet* pcSet);  //One board, or a panel's files, where they are.

	size				NumBoards(void);
	CGerberPanelLoop*	GetOuterLoop(size uiBoard);
	size				NumBoardLoops(size uiBoard);
	CGerberPanelLoop*	GetBoardLoop(size uiBoard, size uiIndex);

	void				Draw(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd);
	void				Arc(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd, SGerberPoint* psCenter, bool bClockwise);

protected:
	bool				ReadBoard(CGerberFileSet* pcSet);
	void				ToPanel(CGerberGraphicsState* pcState, SGerberPoint* psPoint, SFloat64Vec2* psPanel);
	bool				JoinPieces(void);
	bool				Error(const char* szError);
};


void ReverseGerberPanelSegment(SGerberPanelSegment* psSegment);
void IncludeGerberPanelSegmentBounds(SGerberPanelSegment* psSegment, CFloat64Rect* pcBounds, bool* pbFirst);


#endif // __GERBER_PANEL_OUTLINES_H__
