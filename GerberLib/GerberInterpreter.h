#ifndef __GERBER_INTERPRETER_H__
#define __GERBER_INTERPRETER_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "GerberCommands.h"
#include "GerberApertures.h"
#include "GerberGraphicsState.h"
#include "GerberInterpreterListener.h"


typedef CArrayTemplatePtr<CGerberCommandApertureMacro>	CArrayGerberApertureMacroPtr;


//Executes parsed commands: keeps the graphics state and the aperture dictionary, evaluates aperture
//macros and reports the operations, with resolved coordinates, to a listener.
class CGerberInterpreter
{
protected:
	CGerberCommands*				mpcCommands;
	CGerberInterpreterListener*		mpcListener;
	CGerberGraphicsState			mcState;
	CGerberApertures				mcApertures;
	CArrayGerberApertureMacroPtr	mapcMacros;
	bool							mbEndOfFile;

public:
	void							Init(CGerberCommands* pcCommands, CGerberInterpreterListener* pcListener);
	void							Kill(void);

	bool							Interpret(void);

	CGerberGraphicsState*			GetGraphicsState(void);
	CGerberApertures*				GetApertures(void);
	CGerberCommandApertureMacro*	GetMacro(char* szName);

protected:
	bool	InterpretCommands(CGerberCommands* pcCommands);
	bool	InterpretCommand(CGerberCommand* pcCommand);

	bool	InterpretMeasurementMode(CGerberCommandMeasurementMode* pcMeasurementMode);
	bool	InterpretFormatSpecifier(CGerberCommandFormatSpecifier* pcFormatSpecifier);
	bool	InterpretApertureMacro(CGerberCommandApertureMacro* pcMacro);
	bool	InterpretApertureDefine(CGerberCommandApertureDefine* pcDefine);
	bool	InterpretSetAperture(CGerberCommandSetAperture* pcSetAperture);
	bool	InterpretPlotMode(CGerberCommandPlotMode* pcPlotMode);
	bool	InterpretOperation(CGerberCommandOperation* pcOperation);
	bool	InterpretPlot(CGerberCommandOperation* pcOperation, SGerberPoint* psPoint);
	bool	InterpretRegion(CGerberCommandRegion* pcRegion);
	bool	InterpretApertureBlock(CGerberCommandApertureBlock* pcBlock);
	bool	InterpretStepAndRepeat(CGerberCommandStepAndRepeat* pcStepAndRepeat);

	bool	ValidateStandardAperture(CGerberAperture* pcAperture);
	bool	EvaluateMacro(CGerberAperture* pcAperture);
	bool	ResolvePoint(CGerberCommandOperation* pcOperation, SGerberPoint* psPoint);

	bool	Error(const char* szError);
	bool	DCodeError(const char* szLeft, uint32 uiDCode, const char* szRight);
};


#endif // __GERBER_INTERPRETER_H__

