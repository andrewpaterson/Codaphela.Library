#ifndef __GERBER_LAYER_MERGER_H__
#define __GERBER_LAYER_MERGER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Number.h"
#include "GerberWriter.h"
#include "GerberPanel.h"


struct SGerberDCodeMap
{
	uint32	uiOld;
	uint32	uiNew;
};


typedef CArrayTemplate<SGerberDCodeMap>	CArrayGerberDCodeMap;


//Merges one layer of every board on a panel into one Gerber file using the panel's placements.  Each board's file is
//read again and its commands moved, and turned 90 degrees counterclockwise if the board is rotated, exactly.
//D-codes are renumbered across the panel; identical aperture macros are written once and different macros with the
//same name are renamed.  A rotated board's apertures turn with it: R and O swap their sizes, P adds 90 degrees and
//macros are copied as <name>R90 with 90 degrees added to every primitive's rotation.
//Every operation is written with both coordinates so boards don't depend on each other's current point.
//All boards must be in millimetres with the same format.  Step and repeat, block apertures and, on rotated boards,
//LM, LR, LS and single quadrant mode are not supported.
class CGerberLayerMerger
{
protected:
	CGerberPanel*			mpcPanel;
	CGerberWriter			mcWriter;
	CCalculatorSymbols		mcSymbols;  //For the rotated macro expressions.
	CChars*					mpsz;
	uint32					muiNextDCode;
	CArrayChars				maszMacroNames;  //Macros written so far and their bodies.
	CArrayChars				maszMacroBodies;
	uint16					muiXWholes;
	uint16					muiXDecimals;
	uint16					muiYWholes;
	uint16					muiYDecimals;

	//The board being merged.
	CGerberPanelPlacement*	mpcPlacement;
	CNumber					mcOffsetX;
	CNumber					mcOffsetY;
	CArrayGerberDCodeMap	masDCodes;
	CArrayChars				maszOldMacroNames;
	CArrayChars				maszNewMacroNames;
	bool					mbPoint;
	CNumber					mcPointX;  //The current point in the board's own coordinates.
	CNumber					mcPointY;

public:
	void	Init(CGerberPanel* pcPanel);
	void	Kill(void);

	bool	Merge(EGerberBoardLayer eLayer, EGerberBoardSide eSide, CChars* psz);

protected:
	void	Reset(void);
	bool	LoadLayer(CGerberFileSet* pcSet, CGerberLayerFile* pcSource, CGerberLayerFile* pcDest);
	bool	WriteHeader(CGerberCommands* pcCommands);
	bool	WriteBoard(CGerberPanelPlacement* pcPlacement, CGerberCommands* pcCommands);
	bool	TransformCommand(CGerberCommand* pcCommand);
	bool	TransformCommands(CGerberCommands* pcCommands);
	bool	TransformOperation(CGerberCommandOperation* pcOperation);
	bool	TransformApertureDefine(CGerberCommandApertureDefine* pcDefine);
	bool	RotateMacro(CGerberCommandApertureMacro* pcMacro);
	bool	RotateExpression(CGerberExpression* pcExpression);
	bool	MergeMacro(CGerberCommandApertureMacro* pcMacro, bool* pbWrite);
	bool	IsMacroName(char* szName, CArrayChars* paszNames);
	void	WriteText(const char* szText);
	bool	Error(const char* szError);
};


#endif // __GERBER_LAYER_MERGER_H__
