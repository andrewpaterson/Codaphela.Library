#include "BaseLib/Logger.h"
#include "ExcellonWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonWriter::Init(void)
{
	mpsz = NULL;
	mbInches = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonWriter::Kill(void)
{
	mpsz = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonWriter::Write(CExcellonCommands* pcCommands, CChars* psz)
{
	size				i;
	size				uiNumCommands;
	CExcellonCommand*	pcCommand;

	mpsz = psz;

	//The unit is in the header so it is known before any coordinate.
	mbInches = false;
	uiNumCommands = pcCommands->NumCommands();
	for (i = 0; i < uiNumCommands; i++)
	{
		pcCommand = pcCommands->GetCommand(i);
		if (pcCommand->IsType(EC_Unit))
		{
			mbInches = ((CExcellonCommandUnit*)pcCommand)->IsInches();
			break;
		}
	}

	for (i = 0; i < uiNumCommands; i++)
	{
		if (!WriteCommand(pcCommands->GetCommand(i)))
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
bool CExcellonWriter::WriteCommand(CExcellonCommand* pcCommand)
{
	CExcellonCommandToolDefine*	pcToolDefine;
	CExcellonCommandCoordinate*	pcCoordinate;
	CExcellonCommandSlot*		pcSlot;

	switch (pcCommand->GetType())
	{
		case EC_Comment:
			WriteComment((CExcellonCommandComment*)pcCommand);
			break;
		case EC_FileAttribute:
			WriteAttribute("TF", (CExcellonCommandAttribute*)pcCommand);
			break;
		case EC_ApertureAttribute:
			WriteAttribute("TA", (CExcellonCommandAttribute*)pcCommand);
			break;
		case EC_DeleteAttribute:
			WriteAttribute("TD", (CExcellonCommandAttribute*)pcCommand);
			break;
		case EC_HeaderStart:
			mpsz->Append("M48");
			break;
		case EC_Format:
			mpsz->Append("FMAT,");
			mpsz->Append((uint32)((CExcellonCommandFormat*)pcCommand)->GetFormat());
			break;
		case EC_Unit:
			mpsz->Append("METRIC");
			break;
		case EC_ToolDefine:
			pcToolDefine = (CExcellonCommandToolDefine*)pcCommand;
			mpsz->Append('T');
			mpsz->Append(pcToolDefine->GetTool());
			mpsz->Append('C');
			WriteNumber(pcToolDefine->GetDiameter());
			break;
		case EC_HeaderEnd:
			mpsz->Append('%');
			break;
		case EC_Absolute:
			mpsz->Append("G90");
			break;
		case EC_DrillMode:
			mpsz->Append("G05");
			break;
		case EC_ToolSelect:
			mpsz->Append('T');
			mpsz->Append(((CExcellonCommandToolSelect*)pcCommand)->GetTool());
			break;
		case EC_Hit:
		case EC_RouteMove:
		case EC_RouteLinear:
			pcCoordinate = (CExcellonCommandCoordinate*)pcCommand;
			if (pcCommand->IsType(EC_RouteMove))
			{
				mpsz->Append("G00");
			}
			else if (pcCommand->IsType(EC_RouteLinear))
			{
				mpsz->Append("G01");
			}
			WriteCoordinates(pcCoordinate->GetX(), pcCoordinate->HasX(), pcCoordinate->GetY(), pcCoordinate->HasY());
			break;
		case EC_ToolDown:
			mpsz->Append("M15");
			break;
		case EC_ToolUp:
			mpsz->Append("M16");
			break;
		case EC_Slot:
			pcSlot = (CExcellonCommandSlot*)pcCommand;
			WriteCoordinates(pcSlot->GetStartX(), pcSlot->HasStartX(), pcSlot->GetStartY(), pcSlot->HasStartY());
			mpsz->Append("G85");
			WriteCoordinates(pcSlot->GetEndX(), pcSlot->HasEndX(), pcSlot->GetEndY(), pcSlot->HasEndY());
			break;
		case EC_EndOfProgram:
			mpsz->Append("M30");
			break;
		default:
			return Error("Unknown Excellon command.");
	}

	mpsz->AppendNewLine();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonWriter::WriteComment(CExcellonCommandComment* pcComment)
{
	CChars	sz;

	//KiCad describes the format in a comment; keep it true to the metric output.
	sz.Init(pcComment->Text());
	if (mbInches && sz.StartsWith("FORMAT={"))
	{
		sz.Replace("/ inch /", "/ metric /");
	}
	mpsz->Append("; ");
	mpsz->Append(&sz);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonWriter::WriteAttribute(const char* szCode, CExcellonCommandAttribute* pcAttribute)
{
	size	i;
	size	uiNumValues;

	mpsz->Append("; #@! ");
	mpsz->Append(szCode);
	mpsz->Append(pcAttribute->NameText());
	uiNumValues = pcAttribute->NumValues();
	for (i = 0; i < uiNumValues; i++)
	{
		mpsz->Append(',');
		mpsz->Append(pcAttribute->ValueText(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonWriter::WriteCoordinates(CNumber* pcX, bool bX, CNumber* pcY, bool bY)
{
	if (bX)
	{
		mpsz->Append('X');
		WriteNumber(pcX);
	}
	if (bY)
	{
		mpsz->Append('Y');
		WriteNumber(pcY);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonWriter::WriteNumber(CNumber* pcNumber)
{
	CNumber		cMillimetres;
	CNumber		cInchToMillimetres;
	size		uiStart;

	cMillimetres.Init(pcNumber);
	if (mbInches)
	{
		cMillimetres.Multiply(cInchToMillimetres.Init("25.4"));
	}

	//Decimal format always has a decimal point.
	uiStart = mpsz->Length();
	if (cMillimetres.IsZero())
	{
		mpsz->Append('0');
	}
	else
	{
		cMillimetres.Print(mpsz);
	}
	if (mpsz->Find(uiStart, '.') == ARRAY_ELEMENT_NOT_FOUND)
	{
		mpsz->Append(".0");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonWriter::Error(const char* szError)
{
	gcLogger.Error2(__METHOD__, " ", szError, NULL);
	return false;
}

