#ifndef __GERBER_COMMANDS_H__
#define __GERBER_COMMANDS_H__
#include "BaseLib/ListVariable.h"
#include "GerberCommandComment.h"
#include "GerberCommandMeasurementMode.h"
#include "GerberCommandFileAttribute.h"
#include "GerberCommandFormatSpecifier.h"
#include "GerberCommandLoadPolarity.h"
#include "GerberCommandLoadMirroring.h"
#include "GerberCommandPlotMode.h"
#include "GerberCommandApertureMacro.h"
#include "GerberCommandEndOfFile.h"
#include "GerberCommandLoadRotation.h"
#include "GerberCommandLoadScaling.h"


class CGerberCommands
{
protected:
	CListVariable	macCommands;

public:
	void							Init(void);
	void							Kill(void);

	CGerberCommandComment*			AddComment(size uiCommentLength);
	CGerberCommandMeasurementMode*	AddModeSet(EGerberMeasurementMode eMode);
	CGerberCommandFileAttribute*	AddFileAttribute(size iNameLength);
	CGerberCommandFormatSpecifier*  AddFormatSpecifier(uint16 uiXWholes, uint16 uiXDecimals, uint16 uiYWholes, uint16 uiYDecimals);
	CGerberCommandLoadPolarity*		AddLoadPolarity(EGerberPolarity ePolarity);
	CGerberCommandLoadMirroring*	AddLoadMirroring(EGerberMirroring eMirroring);
	CGerberCommandLoadRotation*		AddLoadRotation(CNumber* pcDegrees);
	CGerberCommandLoadScaling*		AddLoadScaling(CNumber* pcScaling);
	CGerberCommandPlotMode*			AddPlotMode(EGerberPlotMode eMode);
	CGerberCommandApertureMacro*	AddApertureMacro(size iNameLength);
	CGerberCommandEndOfFile*		AddEndOfFile(void);
	
	size							NumCommands(void);
	CGerberCommand*					GetCommand(size uiIndex);
};


#endif // __GERBER_COMMANDS_H__

