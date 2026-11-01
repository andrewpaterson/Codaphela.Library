#ifndef __EXCELLON_INTERPRETER_H__
#define __EXCELLON_INTERPRETER_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "ExcellonCommands.h"
#include "ExcellonInterpreterListener.h"


typedef CArrayTemplatePtr<CExcellonCommandToolDefine>	CArrayExcellonToolDefinePtr;


//Executes parsed Excellon commands; keeps the tool table, the current tool and the previous hit and reports holes to a listener.
class CExcellonInterpreter
{
protected:
	CExcellonCommands*				mpcCommands;
	CExcellonInterpreterListener*	mpcListener;

	EExcellonUnit					meUnit;
	CArrayExcellonToolDefinePtr		mapcTools;
	CExcellonCommandToolDefine*		mpcCurrentTool;
	bool							mbAbsolute;
	bool							mbPoint;
	SGerberPoint					msPoint;		//The previous position; modal coordinates are taken from it.

	bool							mbRouteMode;	//G00 to G05.
	bool							mbLinear;		//G01 rather than G00 for coordinates in route mode.
	bool							mbToolDown;		//M15 to M16.

public:
	void						Init(CExcellonCommands* pcCommands, CExcellonInterpreterListener* pcListener);
	void						Kill(void);

	bool						Interpret(void);

	EExcellonUnit				GetUnit(void);
	size						NumTools(void);
	CExcellonCommandToolDefine*	GetTool(uint32 uiTool);

protected:
	bool	InterpretCommand(CExcellonCommand* pcCommand);
	bool	InterpretToolDefine(CExcellonCommandToolDefine* pcToolDefine);
	bool	InterpretToolSelect(CExcellonCommandToolSelect* pcToolSelect);
	bool	InterpretHit(CExcellonCommandCoordinate* pcHit);
	bool	InterpretRouteMove(CExcellonCommandCoordinate* pcMove);
	bool	InterpretRouteLinear(CExcellonCommandCoordinate* pcLinear);
	bool	InterpretToolDown(void);
	bool	InterpretToolUp(void);
	bool	InterpretDrillMode(void);
	bool	InterpretSlot(CExcellonCommandSlot* pcSlot);
	bool	InterpretRoute(void);

	bool	ValidateMachining(void);
	bool	ResolvePoint(CNumber* pcX, bool bX, CNumber* pcY, bool bY, SGerberPoint* psPoint);

	bool	Error(const char* szError);
	bool	ToolError(const char* szLeft, uint32 uiTool, const char* szRight);
};


#endif // __EXCELLON_INTERPRETER_H__

