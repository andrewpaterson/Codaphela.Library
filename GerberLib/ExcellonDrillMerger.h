#ifndef __EXCELLON_DRILL_MERGER_H__
#define __EXCELLON_DRILL_MERGER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/Number.h"
#include "BaseLib/Float64Vec2.h"
#include "ExcellonInterpreterListener.h"
#include "GerberPanel.h"


struct SExcellonMergeHole
{
	CNumber	cX;
	CNumber	cY;
};


struct SExcellonMergeSlot
{
	CNumber	cStartX;
	CNumber	cStartY;
	CNumber	cEndX;
	CNumber	cEndY;
};


typedef CArrayTemplate<SExcellonMergeHole>	CArrayExcellonMergeHole;
typedef CArrayTemplate<SExcellonMergeSlot>	CArrayExcellonMergeSlot;


//One tool of the merged file: every board's tool with the same diameter and function.
class CExcellonMergeTool
{
public:
	CNumber					mcDiameter;  //Millimetres.
	CArrayChars				maszFunction;  //The TA.AperFunction values, eg: Plated, PTH, ViaDrill.
	CChars					mszFunction;  //The values joined by commas.
	CArrayExcellonMergeHole	masHoles;
	CArrayExcellonMergeSlot	masSlots;

	void	Init(CNumber* pcDiameter, CArrayChars* paszFunction);
	void	Kill(void);
};


typedef CArrayTemplatePtr<CExcellonMergeTool>	CArrayExcellonMergeToolPtr;


struct SExcellonBoardTool
{
	uint32	uiTool;
	size	uiFunction;  //Index into the board's functions.
};


typedef CArrayTemplate<SExcellonBoardTool>	CArrayExcellonBoardTool;


//Merges one kind of drill file (plated or non-plated) of every board on a panel into one millimetre Excellon file.
//Tools with the same diameter and function become one tool and the holes of each tool are drilled together.
//Holes that belong to the panel rather than a board, eg: mouse bites, can be added in panel coordinates.
class CExcellonDrillMerger : public CExcellonInterpreterListener
{
protected:
	CGerberPanel*				mpcPanel;
	CArrayExcellonMergeToolPtr	mapcTools;
	CArrayChars					maszFileFunction;  //The first board's TF.FileFunction values.

	//The board being merged.
	CGerberPanelPlacement*		mpcPlacement;
	bool						mbInches;
	CNumber						mcOffsetX;
	CNumber						mcOffsetY;
	CArrayExcellonBoardTool		masBoardTools;
	CArrayChars					maszBoardFunctions;
	bool						mbError;

public:
	void	Init(CGerberPanel* pcPanel);
	void	Kill(void);

	bool	AddBoards(EGerberDrillPlating ePlating);
	void	AddHoles(CNumber* pcDiameter, CArrayChars* paszFunction, SFloat64Vec2* asHoles, size uiNumHoles);
	bool	Write(CChars* psz);

	size	NumTools(void);
	size	NumHoles(void);
	size	NumSlots(void);

	void	Hole(CExcellonCommandToolDefine* pcTool, SGerberPoint* psPoint);
	void	Slot(CExcellonCommandToolDefine* pcTool, SGerberPoint* psStart, SGerberPoint* psEnd);

protected:
	bool				AddBoard(CGerberPanelPlacement* pcPlacement, CExcellonCommands* pcCommands);
	void				ReadBoardTools(CExcellonCommands* pcCommands);
	CExcellonMergeTool*	GetTool(CNumber* pcDiameter, CArrayChars* paszFunction);
	CExcellonMergeTool*	GetBoardTool(CExcellonCommandToolDefine* pcTool);
	void				ToPanel(CNumber* pcX, CNumber* pcY, CNumber* pcDestX, CNumber* pcDestY);
	void				SortTools(void);
	bool				Error(const char* szError);
};


#endif // __EXCELLON_DRILL_MERGER_H__
