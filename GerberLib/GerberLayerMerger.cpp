#include <stdio.h>
#include <stdlib.h>
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/IntegerHelper.h"
#include "GerberApertureMacroCircle.h"
#include "GerberApertureMacroVectorLine.h"
#include "GerberApertureMacroCenterLine.h"
#include "GerberApertureMacroOutline.h"
#include "GerberApertureMacroPolygon.h"
#include "GerberApertureMacroThermal.h"
#include "GerberParser.h"
#include "GerberLayerMerger.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberLayerMerger::Init(CGerberPanel* pcPanel)
{
	mpcPanel = pcPanel;
	mcWriter.Init();
	InitGerberCalculatorSymbols(&mcSymbols);
	mpsz = NULL;
	maszMacroNames.Init();
	maszMacroBodies.Init();
	masDCodes.Init();
	maszOldMacroNames.Init();
	maszNewMacroNames.Init();
	Reset();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberLayerMerger::Kill(void)
{
	maszNewMacroNames.Kill();
	maszOldMacroNames.Kill();
	masDCodes.Kill();
	maszMacroBodies.Kill();
	maszMacroNames.Kill();
	mcSymbols.Kill();
	mcWriter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberLayerMerger::Reset(void)
{
	muiNextDCode = 10;
	maszMacroNames.ReInit();
	maszMacroBodies.ReInit();
	muiXWholes = 0;
	muiXDecimals = 0;
	muiYWholes = 0;
	muiYDecimals = 0;
	mpcPlacement = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::Merge(EGerberBoardLayer eLayer, EGerberBoardSide eSide, CChars* psz)
{
	size					i;
	CGerberPanelPlacement*	pcPlacement;
	CGerberLayerFile*		pcSource;
	CGerberLayerFile		cLayer;
	bool					bHeader;
	bool					bResult;

	Reset();
	mpsz = psz;
	mcWriter.Begin(psz);

	bHeader = false;
	for (i = 0; i < mpcPanel->NumPlacements(); i++)
	{
		pcPlacement = mpcPanel->GetPlacement(i);
		pcSource = pcPlacement->mpcSet->GetLayer(eLayer, eSide);
		if (pcSource == NULL)
		{
			//Boards without this layer are left out of it.
			continue;
		}

		cLayer.Init();
		bResult = LoadLayer(pcPlacement->mpcSet, pcSource, &cLayer);
		if (bResult && !bHeader)
		{
			bResult = WriteHeader(cLayer.GetCommands());
			bHeader = true;
		}
		if (bResult)
		{
			bResult = WriteBoard(pcPlacement, cLayer.GetCommands());
		}
		cLayer.Kill();
		if (!bResult)
		{
			return false;
		}
	}

	if (!bHeader)
	{
		return Error("No board has the layer.");
	}
	WriteText("M02*");
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::LoadLayer(CGerberFileSet* pcSet, CGerberLayerFile* pcSource, CGerberLayerFile* pcDest)
{
	CChars	szPathName;
	bool	bResult;

	//Read again so the file set's own commands are left alone.
	szPathName.Init(pcSet->GetDirectory());
	szPathName.Append(FILE_SEPARATOR);
	szPathName.Append(pcSource->GetFileName());
	bResult = pcDest->Load(szPathName.Text());
	szPathName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::WriteHeader(CGerberCommands* pcCommands)
{
	size							i;
	CGerberCommand*					pcCommand;
	CGerberCommandFileAttribute*	pcAttribute;
	CGerberCommandFormatSpecifier*	pcFormat;
	CGerberCommandMeasurementMode*	pcMode;

	//The board's function and polarity are the panel's.  Its dates, project and origin are not.
	WriteText("%TF.GenerationSoftware,Codaphela,GerberLib*%");
	pcFormat = NULL;
	pcMode = NULL;
	for (i = 0; i < pcCommands->NumCommands(); i++)
	{
		pcCommand = pcCommands->GetCommand(i);
		if (pcCommand->IsFileAttribute())
		{
			pcAttribute = (CGerberCommandFileAttribute*)pcCommand;
			if ((StringCompare(pcAttribute->NameText(), ".FileFunction") == 0) || (StringCompare(pcAttribute->NameText(), ".FilePolarity") == 0))
			{
				if (!mcWriter.WriteCommand(pcCommand))
				{
					return false;
				}
			}
		}
		else if (pcCommand->IsFormatSpecifier() && (pcFormat == NULL))
		{
			pcFormat = (CGerberCommandFormatSpecifier*)pcCommand;
		}
		else if (pcCommand->IsMeasurementMode() && (pcMode == NULL))
		{
			pcMode = (CGerberCommandMeasurementMode*)pcCommand;
		}
	}
	WriteText("%TF.Part,Array*%");

	if ((pcFormat == NULL) || (pcMode == NULL))
	{
		return Error("A board file has no format or unit.");
	}
	muiXWholes = pcFormat->GetXWholeNumbers();
	muiXDecimals = pcFormat->GetXDecimals();
	muiYWholes = pcFormat->GetYWholeNumbers();
	muiYDecimals = pcFormat->GetYDecimals();
	return mcWriter.WriteCommand(pcFormat) && mcWriter.WriteCommand(pcMode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::WriteBoard(CGerberPanelPlacement* pcPlacement, CGerberCommands* pcCommands)
{
	size							i;
	CGerberCommand*					pcCommand;
	CGerberCommandFormatSpecifier*	pcFormat;
	CChars							szName;
	bool							bWrite;

	mpcPlacement = pcPlacement;
	masDCodes.ReInit();
	maszOldMacroNames.ReInit();
	maszNewMacroNames.ReInit();
	mbPoint = false;

	pcPlacement->GetOffset(&mcOffsetX, &mcOffsetY);

	//The board's name in a comment, then its attributes from earlier boards are cleared.
	szName.Init("G04 Board ");
	szName.Append(pcPlacement->mpcSet->GetName());
	szName.Replace('*', '_');
	szName.Replace('%', '_');
	szName.Append(pcPlacement->mbRotated ? ", rotated 90 degrees*" : "*");
	WriteText(szName.Text());
	szName.Kill();
	WriteText("%TD*%");

	for (i = 0; i < pcCommands->NumCommands(); i++)
	{
		pcCommand = pcCommands->GetCommand(i);
		if (pcCommand->IsFileAttribute() || pcCommand->IsEndOfFile())
		{
			continue;
		}
		if (pcCommand->IsFormatSpecifier())
		{
			pcFormat = (CGerberCommandFormatSpecifier*)pcCommand;
			if ((pcFormat->GetXWholeNumbers() != muiXWholes) || (pcFormat->GetXDecimals() != muiXDecimals) || (pcFormat->GetYWholeNumbers() != muiYWholes) || (pcFormat->GetYDecimals() != muiYDecimals))
			{
				return Error("The boards' files have different formats.");
			}
			continue;
		}
		if (pcCommand->IsMeasurementMode())
		{
			if (!((CGerberCommandMeasurementMode*)pcCommand)->IsMillimeters())
			{
				return Error("Only boards in millimetres can be merged.");
			}
			continue;
		}

		bWrite = true;
		if (pcCommand->IsApertureMacro())
		{
			if (!MergeMacro((CGerberCommandApertureMacro*)pcCommand, &bWrite))
			{
				return false;
			}
		}
		else if (!TransformCommand(pcCommand))
		{
			return false;
		}

		if (bWrite && !mcWriter.WriteCommand(pcCommand))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::TransformCommands(CGerberCommands* pcCommands)
{
	size	i;

	for (i = 0; i < pcCommands->NumCommands(); i++)
	{
		if (!TransformCommand(pcCommands->GetCommand(i)))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::TransformCommand(CGerberCommand* pcCommand)
{
	CGerberCommandSetAperture*	pcSetAperture;
	size						i;
	SGerberDCodeMap*			psMap;

	if (pcCommand->IsOperation())
	{
		return TransformOperation((CGerberCommandOperation*)pcCommand);
	}
	else if (pcCommand->IsApertureDefine())
	{
		return TransformApertureDefine((CGerberCommandApertureDefine*)pcCommand);
	}
	else if (pcCommand->IsSetAperture())
	{
		pcSetAperture = (CGerberCommandSetAperture*)pcCommand;
		for (i = 0; i < masDCodes.NumElements(); i++)
		{
			psMap = masDCodes.Get(i);
			if (psMap->uiOld == pcSetAperture->GetDCode())
			{
				pcSetAperture->SetDCode(psMap->uiNew);
				return true;
			}
		}
		return Error("An aperture is selected before it is defined.");
	}
	else if (pcCommand->IsRegion())
	{
		return TransformCommands(((CGerberCommandCompound*)pcCommand)->GetCommands());
	}
	else if (pcCommand->IsApertureBlock() || pcCommand->IsStepAndRepeat())
	{
		return Error("Block apertures and step and repeat can't be merged.");
	}
	else if (mpcPlacement->mbRotated)
	{
		if (pcCommand->IsLoadMirroring() || pcCommand->IsLoadRotation() || pcCommand->IsLoadScaling())
		{
			return Error("A board with LM, LR or LS can't be rotated.");
		}
		if (pcCommand->IsQuadrantMode() && ((CGerberCommandQuadrantMode*)pcCommand)->IsSingleQuadrant())
		{
			return Error("A board in single quadrant mode can't be rotated.");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::TransformOperation(CGerberCommandOperation* pcOperation)
{
	CNumber	cX;
	CNumber	cY;
	CNumber	cI;
	CNumber	cJ;
	CNumber	cNewX;
	CNumber	cNewY;
	bool	bArc;

	//Missing coordinates are the current point's.
	if (pcOperation->HasX())
	{
		cX.Init(pcOperation->GetX());
	}
	else if (mbPoint)
	{
		cX.Init(&mcPointX);
	}
	else
	{
		return Error("An operation has no X and there is no current point.");
	}
	if (pcOperation->HasY())
	{
		cY.Init(pcOperation->GetY());
	}
	else if (mbPoint)
	{
		cY.Init(&mcPointY);
	}
	else
	{
		return Error("An operation has no Y and there is no current point.");
	}
	mcPointX.Init(&cX);
	mcPointY.Init(&cY);
	mbPoint = true;

	bArc = pcOperation->HasI() || pcOperation->HasJ();
	if (bArc)
	{
		cI.Init(0);
		cJ.Init(0);
		if (pcOperation->HasI())
		{
			cI.Init(pcOperation->GetI());
		}
		if (pcOperation->HasJ())
		{
			cJ.Init(pcOperation->GetJ());
		}
	}

	if (mpcPlacement->mbRotated)
	{
		//(x, y) -> (-y, x), and the same for the arc centre offset.
		cNewX.Init(&cY);
		cNewX.Negate();
		cNewY.Init(&cX);
		if (bArc)
		{
			cX.Init(&cJ);
			cX.Negate();
			pcOperation->SetI(&cX);
			pcOperation->SetJ(&cI);
		}
	}
	else
	{
		cNewX.Init(&cX);
		cNewY.Init(&cY);
		if (bArc)
		{
			pcOperation->SetI(&cI);
			pcOperation->SetJ(&cJ);
		}
	}
	cNewX.Add(&mcOffsetX);
	cNewY.Add(&mcOffsetY);
	pcOperation->SetX(&cNewX);
	pcOperation->SetY(&cNewY);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::TransformApertureDefine(CGerberCommandApertureDefine* pcDefine)
{
	SGerberDCodeMap*	psMap;
	char*				szTemplate;
	CNumber				cTemp;
	CNumber				cNinety;
	size				i;

	psMap = masDCodes.Add();
	psMap->uiOld = pcDefine->GetDCode();
	psMap->uiNew = muiNextDCode;
	muiNextDCode++;
	pcDefine->SetDCode(psMap->uiNew);

	szTemplate = pcDefine->TemplateText();
	if ((StringCompare(szTemplate, "C") == 0) || (StringCompare(szTemplate, "R") == 0) || (StringCompare(szTemplate, "O") == 0) || (StringCompare(szTemplate, "P") == 0))
	{
		if (mpcPlacement->mbRotated)
		{
			if ((szTemplate[0] == 'R') || (szTemplate[0] == 'O'))
			{
				if (pcDefine->NumParameters() >= 2)
				{
					cTemp.Init(pcDefine->GetParameter(0));
					pcDefine->GetParameter(0)->Init(pcDefine->GetParameter(1));
					pcDefine->GetParameter(1)->Init(&cTemp);
				}
			}
			else if (szTemplate[0] == 'P')
			{
				cNinety.Init(90);
				if (pcDefine->NumParameters() >= 3)
				{
					pcDefine->GetParameter(2)->Add(&cNinety);
				}
				else
				{
					pcDefine->AddParameter(&cNinety);
				}
			}
		}
		return true;
	}

	//A macro, renamed if it was merged or rotated.
	for (i = 0; i < maszOldMacroNames.NumElements(); i++)
	{
		if (StringCompare(maszOldMacroNames.Get(i)->Text(), szTemplate) == 0)
		{
			pcDefine->SetTemplate(maszNewMacroNames.Get(i)->Text());
			return true;
		}
	}
	return Error("An aperture uses a macro that isn't defined.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::MergeMacro(CGerberCommandApertureMacro* pcMacro, bool* pbWrite)
{
	CGerberWriter	cWriter;
	CChars			szText;
	CChars			szName;
	char*			szBody;
	size			i;
	int				iSuffix;

	if (mpcPlacement->mbRotated)
	{
		if (!RotateMacro(pcMacro))
		{
			return false;
		}
	}

	//The body is everything after the name.
	szText.Init();
	cWriter.Init();
	cWriter.Begin(&szText);
	if (!cWriter.WriteCommand(pcMacro))
	{
		cWriter.Kill();
		szText.Kill();
		return false;
	}
	cWriter.Kill();
	szBody = strchr(szText.Text(), '*');

	maszOldMacroNames.Add(pcMacro->NameText());

	//An identical macro has already been written.
	for (i = 0; i < maszMacroBodies.NumElements(); i++)
	{
		if (StringCompare(maszMacroBodies.Get(i)->Text(), szBody) == 0)
		{
			maszNewMacroNames.Add(maszMacroNames.Get(i)->Text());
			*pbWrite = false;
			szText.Kill();
			return true;
		}
	}

	//A new macro, renamed if another macro already has its name.
	szName.Init(pcMacro->NameText());
	if (mpcPlacement->mbRotated)
	{
		szName.Append("R90");
	}
	iSuffix = 2;
	while (IsMacroName(szName.Text(), &maszMacroNames))
	{
		szName.Kill();
		szName.Init(pcMacro->NameText());
		if (mpcPlacement->mbRotated)
		{
			szName.Append("R90");
		}
		szName.Append('_');
		szName.Append(iSuffix);
		iSuffix++;
	}
	pcMacro->SetName(szName.Text());
	maszNewMacroNames.Add(szName.Text());
	maszMacroNames.Add(szName.Text());
	maszMacroBodies.Add(szBody);
	szName.Kill();
	szText.Kill();
	*pbWrite = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::IsMacroName(char* szName, CArrayChars* paszNames)
{
	size	i;

	for (i = 0; i < paszNames->NumElements(); i++)
	{
		if (StringCompare(paszNames->Get(i)->Text(), szName) == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::RotateMacro(CGerberCommandApertureMacro* pcMacro)
{
	size					i;
	CGerberApertureMacro*	pcPrimitive;
	CGerberExpression*		pcRotation;

	//Every primitive turns around the macro's origin by its rotation.
	for (i = 0; i < pcMacro->NumPrimitives(); i++)
	{
		pcPrimitive = pcMacro->GetPrimitive(i);
		pcRotation = NULL;
		if (pcPrimitive->IsCircle())
		{
			pcRotation = ((CGerberApertureMacroCircle*)pcPrimitive)->GetRotation();
		}
		else if (pcPrimitive->IsVectorLine())
		{
			pcRotation = ((CGerberApertureMacroVectorLine*)pcPrimitive)->GetRotation();
		}
		else if (pcPrimitive->IsCenterLine())
		{
			pcRotation = ((CGerberApertureMacroCenterLine*)pcPrimitive)->GetRotation();
		}
		else if (pcPrimitive->IsOutline())
		{
			pcRotation = ((CGerberApertureMacroOutline*)pcPrimitive)->GetRotation();
		}
		else if (pcPrimitive->IsPolygon())
		{
			pcRotation = ((CGerberApertureMacroPolygon*)pcPrimitive)->GetRotation();
		}
		else if (pcPrimitive->IsThermal())
		{
			pcRotation = ((CGerberApertureMacroThermal*)pcPrimitive)->GetRotation();
		}

		if (pcRotation && !RotateExpression(pcRotation))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::RotateExpression(CGerberExpression* pcExpression)
{
	CChars		szOld;
	CChars		szNew;
	CNumber		cRotation;
	CNumber		cNinety;
	bool		bResult;

	szNew.Init();
	if (!pcExpression->IsSet())
	{
		//A circle without a rotation.
		szNew.Append("90");
	}
	else if (pcExpression->GetConstNumber())
	{
		cRotation.Init(pcExpression->GetConstNumber());
		cRotation.Add(cNinety.Init(90));
		cRotation.Print(&szNew);
	}
	else
	{
		szOld.Init();
		if (!pcExpression->Print(&szOld))
		{
			szOld.Kill();
			szNew.Kill();
			return Error("A macro rotation could not be printed.");
		}
		szNew.Append('(');
		szNew.Append(&szOld);
		szNew.Append(")+90");
		szOld.Kill();
	}

	bResult = pcExpression->Parse(szNew.Text(), &mcSymbols);
	szNew.Kill();
	if (!bResult)
	{
		return Error("A rotated macro rotation could not be parsed.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberLayerMerger::WriteText(const char* szText)
{
	mpsz->Append(szText);
	mpsz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerMerger::Error(const char* szError)
{
	if (mpcPlacement)
	{
		gcLogger.Error2(__METHOD__, " Board [", mpcPlacement->mpcSet->GetName(), "]: ", szError, NULL);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " ", szError, NULL);
	}
	return false;
}

