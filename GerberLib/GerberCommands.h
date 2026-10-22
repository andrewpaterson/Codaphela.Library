#ifndef __GERBER_COMMANDS_H__
#define __GERBER_COMMANDS_H__
#include "BaseLib/ListVariable.h"
#include "GerberCommandComment.h"
#include "GerberCommandMeasurementMode.h"
#include "GerberCommandFileAttribute.h"
#include "GerberCommandApertureAttribute.h"
#include "GerberCommandObjectAttribute.h"
#include "GerberCommandFormatSpecifier.h"
#include "GerberCommandLoadPolarity.h"
#include "GerberCommandLoadMirroring.h"
#include "GerberCommandPlotMode.h"
#include "GerberCommandApertureMacro.h"
#include "GerberCommandEndOfFile.h"
#include "GerberCommandLoadRotation.h"
#include "GerberCommandLoadScaling.h"
#include "GerberCommandDeleteAttribute.h"
#include "GerberCommandApertureDefine.h"
#include "GerberCommandSetAperture.h"
#include "GerberCommandOperation.h"
#include "GerberCommandQuadrantMode.h"
#include "GerberCommandRegion.h"
#include "GerberCommandApertureBlock.h"
#include "GerberCommandStepAndRepeat.h"


class CGerberCommands
{
protected:
	CListVariable	macCommands;

public:
	void								Init(void);
	void								Kill(void);

	CGerberCommandComment*				AddComment(size uiCommentLength);
	CGerberCommandMeasurementMode*		AddModeSet(EGerberMeasurementMode eMode);
	CGerberCommandFileAttribute*		AddFileAttribute(size iNameLength);
	CGerberCommandApertureAttribute*	AddApertureAttribute(size iNameLength);
	CGerberCommandObjectAttribute*		AddObjectAttribute(size iNameLength);
	CGerberCommandDeleteAttribute*		AddDeleteAttribute(size iNameLength);
	CGerberCommandApertureDefine*		AddApertureDefine(uint32 uiDCode, size iTemplateLength);
	CGerberCommandSetAperture*			AddSetAperture(uint32 uiDCode);
	CGerberCommandOperation*			AddOperation(EGerberCommandEnum eType);
	CGerberCommandFormatSpecifier*		AddFormatSpecifier(uint16 uiXWholes, uint16 uiXDecimals, uint16 uiYWholes, uint16 uiYDecimals);
	CGerberCommandLoadPolarity*			AddLoadPolarity(EGerberPolarity ePolarity);
	CGerberCommandLoadMirroring*		AddLoadMirroring(EGerberMirroring eMirroring);
	CGerberCommandLoadRotation*			AddLoadRotation(CNumber* pcDegrees);
	CGerberCommandLoadScaling*			AddLoadScaling(CNumber* pcScaling);
	CGerberCommandPlotMode*				AddPlotMode(EGerberPlotMode eMode);
	CGerberCommandQuadrantMode*			AddQuadrantMode(EGerberQuadrantMode eMode);
	CGerberCommandRegion*				AddRegion(void);
	CGerberCommandApertureBlock*		AddApertureBlock(uint32 uiDCode);
	CGerberCommandStepAndRepeat*		AddStepAndRepeat(uint32 uiXRepeats, uint32 uiYRepeats, CNumber* pcXStep, CNumber* pcYStep);
	CGerberCommandApertureMacro*		AddApertureMacro(size iNameLength);
	CGerberCommandEndOfFile*			AddEndOfFile(void);
	
	size								NumCommands(void);
	CGerberCommand*						GetCommand(size uiIndex);
};


#endif // __GERBER_COMMANDS_H__

