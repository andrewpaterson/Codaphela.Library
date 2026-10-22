#include <math.h>
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "ExcellonInterpreter.h"
#include "ExcellonWriter.h"
#include "ExcellonParser.h"
#include "ExcellonDrillMerger.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void JoinStrings(CArrayChars* paszStrings, CChars* psz)
{
	size	i;

	for (i = 0; i < paszStrings->NumElements(); i++)
	{
		if (i != 0)
		{
			psz->Append(',');
		}
		psz->Append(paszStrings->Get(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonMergeTool::Init(CNumber* pcDiameter, CArrayChars* paszFunction)
{
	size	i;

	mcDiameter.Init(pcDiameter);
	maszFunction.Init();
	for (i = 0; i < paszFunction->NumElements(); i++)
	{
		maszFunction.Add(paszFunction->Get(i));
	}
	mszFunction.Init();
	JoinStrings(&maszFunction, &mszFunction);
	masHoles.Init();
	masSlots.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonMergeTool::Kill(void)
{
	masSlots.Kill();
	masHoles.Kill();
	mszFunction.Kill();
	maszFunction.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonDrillMerger::Init(CGerberPanel* pcPanel)
{
	mpcPanel = pcPanel;
	mapcTools.Init();
	maszFileFunction.Init();
	mpcPlacement = NULL;
	mbInches = false;
	masBoardTools.Init();
	maszBoardFunctions.Init();
	mbError = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonDrillMerger::Kill(void)
{
	size				i;
	CExcellonMergeTool*	pcTool;

	maszBoardFunctions.Kill();
	masBoardTools.Kill();
	maszFileFunction.Kill();
	for (i = 0; i < mapcTools.NumElements(); i++)
	{
		pcTool = mapcTools.GetPtr(i);
		SafeKill(pcTool);
	}
	mapcTools.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonDrillMerger::AddBoards(EGerberDrillPlating ePlating)
{
	size					i;
	CGerberPanelPlacement*	pcPlacement;
	CGerberDrillFile*		pcSource;
	CGerberDrillFile		cDrill;
	CChars					szPathName;
	bool					bResult;

	for (i = 0; i < mpcPanel->NumPlacements(); i++)
	{
		pcPlacement = mpcPanel->GetPlacement(i);
		pcSource = pcPlacement->mpcSet->GetDrill(ePlating);
		if (pcSource == NULL)
		{
			continue;
		}

		//Read again so the file set's own commands are left alone.
		szPathName.Init(pcPlacement->mpcSet->GetDirectory());
		szPathName.Append(FILE_SEPARATOR);
		szPathName.Append(pcSource->GetFileName());
		cDrill.Init();
		bResult = cDrill.Load(szPathName.Text());
		szPathName.Kill();
		if (bResult)
		{
			bResult = AddBoard(pcPlacement, cDrill.GetCommands());
		}
		cDrill.Kill();
		if (!bResult)
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
bool CExcellonDrillMerger::AddBoard(CGerberPanelPlacement* pcPlacement, CExcellonCommands* pcCommands)
{
	CExcellonInterpreter	cInterpreter;
	bool					bResult;

	mpcPlacement = pcPlacement;
	pcPlacement->GetOffset(&mcOffsetX, &mcOffsetY);
	ReadBoardTools(pcCommands);

	mbError = false;
	cInterpreter.Init(pcCommands, this);
	bResult = cInterpreter.Interpret();
	cInterpreter.Kill();
	if (!bResult)
	{
		return Error("The drill file could not be interpreted.");
	}
	return !mbError;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonDrillMerger::ReadBoardTools(CExcellonCommands* pcCommands)
{
	size						i;
	size						j;
	CExcellonCommand*			pcCommand;
	CExcellonCommandAttribute*	pcAttribute;
	SExcellonBoardTool*			psTool;
	CArrayChars					aszFunction;
	CChars						szFunction;

	//The unit, each tool's TA.AperFunction and the first board's TF.FileFunction.
	mbInches = false;
	masBoardTools.ReInit();
	maszBoardFunctions.ReInit();
	aszFunction.Init();
	for (i = 0; i < pcCommands->NumCommands(); i++)
	{
		pcCommand = pcCommands->GetCommand(i);
		if (pcCommand->IsType(EC_Unit))
		{
			mbInches = ((CExcellonCommandUnit*)pcCommand)->IsInches();
		}
		else if (pcCommand->IsType(EC_ApertureAttribute))
		{
			pcAttribute = (CExcellonCommandAttribute*)pcCommand;
			if (StringCompare(pcAttribute->NameText(), ".AperFunction") == 0)
			{
				aszFunction.ReInit();
				for (j = 0; j < pcAttribute->NumValues(); j++)
				{
					aszFunction.Add(pcAttribute->ValueText(j));
				}
			}
		}
		else if (pcCommand->IsType(EC_DeleteAttribute))
		{
			aszFunction.ReInit();
		}
		else if (pcCommand->IsType(EC_FileAttribute))
		{
			pcAttribute = (CExcellonCommandAttribute*)pcCommand;
			if ((maszFileFunction.NumElements() == 0) && (StringCompare(pcAttribute->NameText(), ".FileFunction") == 0))
			{
				for (j = 0; j < pcAttribute->NumValues(); j++)
				{
					maszFileFunction.Add(pcAttribute->ValueText(j));
				}
			}
		}
		else if (pcCommand->IsType(EC_ToolDefine))
		{
			psTool = masBoardTools.Add();
			psTool->uiTool = ((CExcellonCommandToolDefine*)pcCommand)->GetTool();
			psTool->uiFunction = maszBoardFunctions.NumElements();
			szFunction.Init();
			JoinStrings(&aszFunction, &szFunction);
			maszBoardFunctions.Add(szFunction.Text());
			szFunction.Kill();
		}
	}
	aszFunction.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CExcellonMergeTool* CExcellonDrillMerger::GetBoardTool(CExcellonCommandToolDefine* pcTool)
{
	size				i;
	SExcellonBoardTool*	psTool;
	CNumber				cDiameter;
	CNumber				cInchToMillimetres;
	CArrayChars			aszFunction;
	CExcellonMergeTool*	pcMergeTool;

	cDiameter.Init(pcTool->GetDiameter());
	if (mbInches)
	{
		cDiameter.Multiply(cInchToMillimetres.Init("25.4"));
	}

	aszFunction.Init();
	for (i = 0; i < masBoardTools.NumElements(); i++)
	{
		psTool = masBoardTools.Get(i);
		if (psTool->uiTool == pcTool->GetTool())
		{
			if (!maszBoardFunctions.Get(psTool->uiFunction)->Empty())
			{
				maszBoardFunctions.Get(psTool->uiFunction)->Split(&aszFunction, ',');
			}
			break;
		}
	}
	pcMergeTool = GetTool(&cDiameter, &aszFunction);
	aszFunction.Kill();
	return pcMergeTool;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CExcellonMergeTool* CExcellonDrillMerger::GetTool(CNumber* pcDiameter, CArrayChars* paszFunction)
{
	size				i;
	CExcellonMergeTool*	pcTool;
	CChars				szFunction;

	szFunction.Init();
	JoinStrings(paszFunction, &szFunction);
	for (i = 0; i < mapcTools.NumElements(); i++)
	{
		pcTool = mapcTools.GetPtr(i);
		if (pcTool->mcDiameter.Equals(pcDiameter) && pcTool->mszFunction.Equals(szFunction.Text()))
		{
			szFunction.Kill();
			return pcTool;
		}
	}
	szFunction.Kill();

	pcTool = NewMalloc<CExcellonMergeTool>();
	pcTool->Init(pcDiameter, paszFunction);
	mapcTools.Add(pcTool);
	return pcTool;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonDrillMerger::ToPanel(CNumber* pcX, CNumber* pcY, CNumber* pcDestX, CNumber* pcDestY)
{
	CNumber	cX;
	CNumber	cY;
	CNumber	cInchToMillimetres;

	cX.Init(pcX);
	cY.Init(pcY);
	if (mbInches)
	{
		cInchToMillimetres.Init("25.4");
		cX.Multiply(&cInchToMillimetres);
		cY.Multiply(&cInchToMillimetres);
	}

	//(x, y) -> (-y, x) if rotated, then moved.
	if (mpcPlacement->mbRotated)
	{
		pcDestX->Init(&cY);
		pcDestX->Negate();
		pcDestY->Init(&cX);
	}
	else
	{
		pcDestX->Init(&cX);
		pcDestY->Init(&cY);
	}
	pcDestX->Add(&mcOffsetX);
	pcDestY->Add(&mcOffsetY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonDrillMerger::Hole(CExcellonCommandToolDefine* pcTool, SGerberPoint* psPoint)
{
	CExcellonMergeTool*	pcMergeTool;
	SExcellonMergeHole*	psHole;

	pcMergeTool = GetBoardTool(pcTool);
	psHole = pcMergeTool->masHoles.Add();
	ToPanel(&psPoint->cX, &psPoint->cY, &psHole->cX, &psHole->cY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonDrillMerger::Slot(CExcellonCommandToolDefine* pcTool, SGerberPoint* psStart, SGerberPoint* psEnd)
{
	CExcellonMergeTool*	pcMergeTool;
	SExcellonMergeSlot*	psSlot;

	pcMergeTool = GetBoardTool(pcTool);
	psSlot = pcMergeTool->masSlots.Add();
	ToPanel(&psStart->cX, &psStart->cY, &psSlot->cStartX, &psSlot->cStartY);
	ToPanel(&psEnd->cX, &psEnd->cY, &psSlot->cEndX, &psSlot->cEndY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonDrillMerger::AddHoles(CNumber* pcDiameter, CArrayChars* paszFunction, SFloat64Vec2* asHoles, size uiNumHoles)
{
	CExcellonMergeTool*	pcTool;
	SExcellonMergeHole*	psHole;
	size				i;

	pcTool = GetTool(pcDiameter, paszFunction);
	for (i = 0; i < uiNumHoles; i++)
	{
		psHole = pcTool->masHoles.Add();
		MillimetresToNumber(asHoles[i].x, &psHole->cX);
		MillimetresToNumber(asHoles[i].y, &psHole->cY);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonDrillMerger::SortTools(void)
{
	size				i;
	size				j;
	CExcellonMergeTool*	pcTool;
	CExcellonMergeTool*	pcOther;

	//Smallest drill first, as KiCad writes them.
	for (i = 1; i < mapcTools.NumElements(); i++)
	{
		for (j = i; j > 0; j--)
		{
			pcTool = mapcTools.GetPtr(j);
			pcOther = mapcTools.GetPtr(j - 1);
			if (!pcTool->mcDiameter.LessThan(&pcOther->mcDiameter))
			{
				break;
			}
			mapcTools.SetPtr(j, pcOther);
			mapcTools.SetPtr(j - 1, pcTool);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonDrillMerger::Write(CChars* psz)
{
	CExcellonCommands			cCommands;
	CExcellonWriter				cWriter;
	CExcellonCommandAttribute*	pcAttribute;
	CExcellonCommandCoordinate*	pcHit;
	CExcellonCommandSlot*		pcSlot;
	CExcellonMergeTool*			pcTool;
	SExcellonMergeHole*			psHole;
	SExcellonMergeSlot*			psSlot;
	size						i;
	size						j;
	bool						bResult;
	const char*					szComment;
	const char*					szFormat;

	SortTools();

	cCommands.Init();
	cCommands.AddCode(EC_HeaderStart);
	szComment = "DRILL file {Codaphela GerberLib} panel";
	cCommands.AddComment(szComment, strlen(szComment));
	szFormat = "FORMAT={-:-/ absolute / metric / decimal}";
	cCommands.AddComment(szFormat, strlen(szFormat));
	pcAttribute = cCommands.AddAttribute(EC_FileAttribute, ".GenerationSoftware", strlen(".GenerationSoftware"));
	pcAttribute->AddValue("Codaphela", strlen("Codaphela"));
	pcAttribute->AddValue("GerberLib", strlen("GerberLib"));
	if (maszFileFunction.NumElements() > 0)
	{
		pcAttribute = cCommands.AddAttribute(EC_FileAttribute, ".FileFunction", strlen(".FileFunction"));
		for (i = 0; i < maszFileFunction.NumElements(); i++)
		{
			pcAttribute->AddValue(maszFileFunction.Get(i)->Text(), maszFileFunction.Get(i)->Length());
		}
	}
	cCommands.AddFormat(2);
	cCommands.AddUnit(EU_Millimeters);
	for (i = 0; i < mapcTools.NumElements(); i++)
	{
		pcTool = mapcTools.GetPtr(i);
		if (pcTool->maszFunction.NumElements() > 0)
		{
			pcAttribute = cCommands.AddAttribute(EC_ApertureAttribute, ".AperFunction", strlen(".AperFunction"));
			for (j = 0; j < pcTool->maszFunction.NumElements(); j++)
			{
				pcAttribute->AddValue(pcTool->maszFunction.Get(j)->Text(), pcTool->maszFunction.Get(j)->Length());
			}
		}
		cCommands.AddToolDefine((uint32)(i + 1), &pcTool->mcDiameter);
	}
	cCommands.AddCode(EC_HeaderEnd);
	cCommands.AddCode(EC_Absolute);
	cCommands.AddCode(EC_DrillMode);

	for (i = 0; i < mapcTools.NumElements(); i++)
	{
		pcTool = mapcTools.GetPtr(i);
		cCommands.AddToolSelect((uint32)(i + 1));
		for (j = 0; j < pcTool->masHoles.NumElements(); j++)
		{
			psHole = pcTool->masHoles.Get(j);
			pcHit = cCommands.AddCoordinate(EC_Hit);
			pcHit->SetX(&psHole->cX);
			pcHit->SetY(&psHole->cY);
		}
		for (j = 0; j < pcTool->masSlots.NumElements(); j++)
		{
			psSlot = pcTool->masSlots.Get(j);
			pcSlot = cCommands.AddSlot();
			pcSlot->SetStartX(&psSlot->cStartX);
			pcSlot->SetStartY(&psSlot->cStartY);
			pcSlot->SetEndX(&psSlot->cEndX);
			pcSlot->SetEndY(&psSlot->cEndY);
		}
	}
	cCommands.AddToolSelect(0);
	cCommands.AddCode(EC_EndOfProgram);

	cWriter.Init();
	bResult = cWriter.Write(&cCommands, psz);
	cWriter.Kill();
	cCommands.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CExcellonDrillMerger::NumTools(void)
{
	return mapcTools.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CExcellonDrillMerger::NumHoles(void)
{
	size	i;
	size	uiNumHoles;

	uiNumHoles = 0;
	for (i = 0; i < mapcTools.NumElements(); i++)
	{
		uiNumHoles += mapcTools.GetPtr(i)->masHoles.NumElements();
	}
	return uiNumHoles;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CExcellonDrillMerger::NumSlots(void)
{
	size	i;
	size	uiNumSlots;

	uiNumSlots = 0;
	for (i = 0; i < mapcTools.NumElements(); i++)
	{
		uiNumSlots += mapcTools.GetPtr(i)->masSlots.NumElements();
	}
	return uiNumSlots;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonDrillMerger::Error(const char* szError)
{
	mbError = true;
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

