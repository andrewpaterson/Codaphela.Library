#include "BaseLib/Logger.h"
#include "ExcellonParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonParser::Init(char* szText, size uiTextLength, char* szFileName, CExcellonCommands* pcCommands)
{
	mszText = szText;
	muiTextLength = uiTextLength;
	mszFilename.Init(szFileName);
	mpcCommands = pcCommands;

	muiPosition = 0;
	muiLine = 0;
	mszLine = NULL;
	muiLineLength = 0;

	mbHeaderStarted = false;
	mbHeaderEnded = false;
	mbEndOfProgram = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExcellonParser::Kill(void)
{
	mpcCommands = NULL;
	mszFilename.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CExcellonParser::Parse(void)
{
	TRISTATE	tResult;

	while (NextLine())
	{
		if (muiLineLength == 0)
		{
			continue;
		}

		tResult = ParseLine();
		if (tResult != TRITRUE)
		{
			return tResult;
		}
	}

	if (!mbEndOfProgram)
	{
		return Error("End of program (M30) is missing.");
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonParser::NextLine(void)
{
	size	uiEnd;
	char	c;

	if (muiPosition >= muiTextLength)
	{
		return false;
	}

	mszLine = &mszText[muiPosition];
	uiEnd = muiPosition;
	while ((uiEnd < muiTextLength) && (mszText[uiEnd] != '\n'))
	{
		uiEnd++;
	}
	muiLineLength = uiEnd - muiPosition;
	muiPosition = uiEnd + 1;
	muiLine++;

	//Strip the carriage return and trailing whitespace.
	while (muiLineLength > 0)
	{
		c = mszLine[muiLineLength - 1];
		if ((c == '\r') || (c == ' ') || (c == '\t'))
		{
			muiLineLength--;
		}
		else
		{
			break;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CExcellonParser::ParseLine(void)
{
	if (mszLine[0] == ';')
	{
		return ParseComment();
	}

	if (mbEndOfProgram)
	{
		return Error("Nothing may follow the end of program (M30), could not parse Command:");
	}

	if (!mbHeaderStarted)
	{
		if (!LineIs("M48"))
		{
			return Error("Excellon file must start with M48, could not parse Command:");
		}
		mpcCommands->AddCode(EC_HeaderStart);
		mbHeaderStarted = true;
		return TRITRUE;
	}

	if (!mbHeaderEnded)
	{
		return ParseHeaderLine();
	}
	else
	{
		return ParseBodyLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CExcellonParser::ParseComment(void)
{
	size						uiStart;
	size						uiEnd;
	CExcellonCommandAttribute*	pcAttribute;
	EExcellonCommandEnum		eType;

	//"; #@! TD" deletes the aperture attributes.
	if (LineIs("; #@! TD") || LineStartsWith("; #@! TD."))
	{
		mpcCommands->AddAttribute(EC_DeleteAttribute, &mszLine[8], muiLineLength - 8);
		return TRITRUE;
	}

	//Gerber X2 attributes are written as "; #@! TF.Name,Value,Value".
	if (LineStartsWith("; #@! TF.") || LineStartsWith("; #@! TA."))
	{
		eType = (mszLine[7] == 'F') ? EC_FileAttribute : EC_ApertureAttribute;

		uiStart = 8;
		uiEnd = uiStart;
		while ((uiEnd < muiLineLength) && (mszLine[uiEnd] != ','))
		{
			uiEnd++;
		}
		pcAttribute = mpcCommands->AddAttribute(eType, &mszLine[uiStart], uiEnd - uiStart);

		while (uiEnd < muiLineLength)
		{
			uiStart = uiEnd + 1;
			uiEnd = uiStart;
			while ((uiEnd < muiLineLength) && (mszLine[uiEnd] != ','))
			{
				uiEnd++;
			}
			pcAttribute->AddValue(&mszLine[uiStart], uiEnd - uiStart);
		}
		return TRITRUE;
	}

	uiStart = 1;
	if ((uiStart < muiLineLength) && (mszLine[uiStart] == ' '))
	{
		uiStart++;
	}
	mpcCommands->AddComment(&mszLine[uiStart], muiLineLength - uiStart);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CExcellonParser::ParseHeaderLine(void)
{
	if (LineIs("%") || LineIs("M95"))
	{
		mpcCommands->AddCode(EC_HeaderEnd);
		mbHeaderEnded = true;
		return TRITRUE;
	}
	else if (LineIs("FMAT,1"))
	{
		mpcCommands->AddFormat(1);
		return TRITRUE;
	}
	else if (LineIs("FMAT,2"))
	{
		mpcCommands->AddFormat(2);
		return TRITRUE;
	}
	else if (LineIs("INCH"))
	{
		mpcCommands->AddUnit(EU_Inches);
		return TRITRUE;
	}
	else if (LineIs("METRIC"))
	{
		mpcCommands->AddUnit(EU_Millimeters);
		return TRITRUE;
	}
	else if (LineStartsWith("INCH,") || LineStartsWith("METRIC,"))
	{
		//Leading or trailing zero suppression (LZ, TZ) and number formats need coordinates without a decimal point.
		return Error("Excellon zero suppression and number formats are not supported, could not parse Command:");
	}
	else if (mszLine[0] == 'T')
	{
		return ParseToolDefine();
	}
	return Error("Unknown Excellon header command, could not parse Command:");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CExcellonParser::ParseBodyLine(void)
{
	if (LineIs("G90"))
	{
		mpcCommands->AddCode(EC_Absolute);
		return TRITRUE;
	}
	else if (LineIs("G91"))
	{
		return Error("Excellon incremental coordinates (G91) are not supported, could not parse Command:");
	}
	else if (LineIs("G05"))
	{
		mpcCommands->AddCode(EC_DrillMode);
		return TRITRUE;
	}
	else if (LineIs("M30"))
	{
		mpcCommands->AddCode(EC_EndOfProgram);
		mbEndOfProgram = true;
		return TRITRUE;
	}
	else if (mszLine[0] == 'T')
	{
		return ParseToolSelect();
	}
	else if ((mszLine[0] == 'X') || (mszLine[0] == 'Y'))
	{
		return ParseCoordinate(EC_Hit, 0);
	}
	else if (LineStartsWith("G00"))
	{
		return ParseCoordinate(EC_RouteMove, 3);
	}
	else if (LineStartsWith("G01"))
	{
		return ParseCoordinate(EC_RouteLinear, 3);
	}
	else if (LineIs("M15"))
	{
		mpcCommands->AddCode(EC_ToolDown);
		return TRITRUE;
	}
	else if (LineIs("M16"))
	{
		mpcCommands->AddCode(EC_ToolUp);
		return TRITRUE;
	}
	else if (LineStartsWith("G02") || LineStartsWith("G03"))
	{
		return Error("Excellon circular routing (G02, G03) is not supported, could not parse Command:");
	}
	return Error("Unknown Excellon command, could not parse Command:");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CExcellonParser::ParseToolDefine(void)
{
	size		uiPosition;
	uint32		uiTool;
	CNumber		cDiameter;

	//TnCd
	uiPosition = 1;
	if (!GetUnsigned(&uiPosition, &uiTool))
	{
		return Error("Excellon tool number expected, could not parse Command:");
	}
	if (uiTool == 0)
	{
		return Error("Excellon tool 0 cannot be defined, could not parse Command:");
	}

	if ((uiPosition >= muiLineLength) || (mszLine[uiPosition] != 'C'))
	{
		return Error("Excellon tool diameter (C) expected, could not parse Command:");
	}
	uiPosition++;
	if (!GetNumber(&uiPosition, &cDiameter, false) || cDiameter.IsNegative())
	{
		return Error("Excellon tool diameter expected, could not parse Command:");
	}
	if (uiPosition != muiLineLength)
	{
		return Error("Unexpected characters after Excellon tool definition, could not parse Command:");
	}

	mpcCommands->AddToolDefine(uiTool, &cDiameter);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CExcellonParser::ParseToolSelect(void)
{
	size		uiPosition;
	uint32		uiTool;

	uiPosition = 1;
	if (!GetUnsigned(&uiPosition, &uiTool))
	{
		return Error("Excellon tool number expected, could not parse Command:");
	}
	if (uiPosition != muiLineLength)
	{
		return Error("Unexpected characters after Excellon tool selection, could not parse Command:");
	}

	mpcCommands->AddToolSelect(uiTool);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CExcellonParser::ParseCoordinates(size* puiPosition, CNumber* pcX, bool* pbX, CNumber* pcY, bool* pbY)
{
	*pbX = false;
	*pbY = false;
	if ((*puiPosition < muiLineLength) && (mszLine[*puiPosition] == 'X'))
	{
		(*puiPosition)++;
		if (!GetNumber(puiPosition, pcX, true))
		{
			return Error("Excellon X coordinate with a decimal point expected, could not parse Command:");
		}
		*pbX = true;
	}
	if ((*puiPosition < muiLineLength) && (mszLine[*puiPosition] == 'Y'))
	{
		(*puiPosition)++;
		if (!GetNumber(puiPosition, pcY, true))
		{
			return Error("Excellon Y coordinate with a decimal point expected, could not parse Command:");
		}
		*pbY = true;
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CExcellonParser::ParseCoordinate(EExcellonCommandEnum eType, size uiPosition)
{
	CNumber						cX;
	CNumber						cY;
	bool						bX;
	bool						bY;
	CNumber						cEndX;
	CNumber						cEndY;
	bool						bEndX;
	bool						bEndY;
	CExcellonCommandCoordinate*	pcCoordinate;
	CExcellonCommandSlot*		pcSlot;
	TRISTATE					tResult;

	tResult = ParseCoordinates(&uiPosition, &cX, &bX, &cY, &bY);
	if (tResult != TRITRUE)
	{
		return tResult;
	}
	if (!bX && !bY)
	{
		return Error("Excellon coordinates expected, could not parse Command:");
	}

	if ((eType == EC_Hit) && (uiPosition + 3 <= muiLineLength) && (memcmp(&mszLine[uiPosition], "G85", 3) == 0))
	{
		//A canned slot; XxYyG85XxYy.
		uiPosition += 3;
		tResult = ParseCoordinates(&uiPosition, &cEndX, &bEndX, &cEndY, &bEndY);
		if (tResult != TRITRUE)
		{
			return tResult;
		}
		if (!bEndX && !bEndY)
		{
			return Error("Excellon slot (G85) end coordinates expected, could not parse Command:");
		}
		if (uiPosition != muiLineLength)
		{
			return Error("Unexpected characters after Excellon coordinates, could not parse Command:");
		}

		pcSlot = mpcCommands->AddSlot();
		if (bX)
		{
			pcSlot->SetStartX(&cX);
		}
		if (bY)
		{
			pcSlot->SetStartY(&cY);
		}
		if (bEndX)
		{
			pcSlot->SetEndX(&cEndX);
		}
		if (bEndY)
		{
			pcSlot->SetEndY(&cEndY);
		}
		return TRITRUE;
	}

	if (uiPosition != muiLineLength)
	{
		return Error("Unexpected characters after Excellon coordinates, could not parse Command:");
	}

	pcCoordinate = mpcCommands->AddCoordinate(eType);
	if (bX)
	{
		pcCoordinate->SetX(&cX);
	}
	if (bY)
	{
		pcCoordinate->SetY(&cY);
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonParser::LineIs(const char* szText)
{
	size	uiLength;

	uiLength = strlen(szText);
	return (uiLength == muiLineLength) && (memcmp(mszLine, szText, uiLength) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonParser::LineStartsWith(const char* szText)
{
	size	uiLength;

	uiLength = strlen(szText);
	return (uiLength <= muiLineLength) && (memcmp(mszLine, szText, uiLength) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonParser::GetUnsigned(size* puiPosition, uint32* puiValue)
{
	size	uiPosition;
	uint64	ulliValue;

	uiPosition = *puiPosition;
	ulliValue = 0;
	while ((uiPosition < muiLineLength) && (mszLine[uiPosition] >= '0') && (mszLine[uiPosition] <= '9'))
	{
		ulliValue = ulliValue * 10 + (mszLine[uiPosition] - '0');
		if (ulliValue > MAX_UINT32)
		{
			return false;
		}
		uiPosition++;
	}

	if (uiPosition == *puiPosition)
	{
		return false;
	}
	*puiValue = (uint32)ulliValue;
	*puiPosition = uiPosition;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExcellonParser::GetNumber(size* puiPosition, CNumber* pcNumber, bool bNeedPoint)
{
	size	uiPosition;
	size	uiDigits;
	bool	bPoint;
	char	c;

	//A decimal number; [+-]digits[.digits].  KiCad's decimal format always writes the decimal point in coordinates;
	//coordinates without one are in a zero format (keep, leading or trailing zeros) with an implied decimal point.
	uiPosition = *puiPosition;
	if ((uiPosition < muiLineLength) && ((mszLine[uiPosition] == '-') || (mszLine[uiPosition] == '+')))
	{
		uiPosition++;
	}

	uiDigits = 0;
	bPoint = false;
	while (uiPosition < muiLineLength)
	{
		c = mszLine[uiPosition];
		if ((c >= '0') && (c <= '9'))
		{
			uiDigits++;
		}
		else if ((c == '.') && !bPoint)
		{
			bPoint = true;
		}
		else
		{
			break;
		}
		uiPosition++;
	}

	if ((uiDigits == 0) || (bNeedPoint && !bPoint))
	{
		return false;
	}

	pcNumber->Init(&mszLine[*puiPosition], uiPosition - *puiPosition);
	if (pcNumber->IsNAN())
	{
		return false;
	}
	*puiPosition = uiPosition;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CExcellonParser::Error(const char* szError)
{
	CChars	sz;

	sz.Init(mszFilename);
	sz.Append('(');
	sz.Append(muiLine);
	sz.Append("): ");
	sz.Append(szError);
	sz.AppendNewLine();
	sz.Append(mszLine, muiLineLength);
	gcLogger.Error2(__METHOD__, " ", sz.Text(), NULL);
	sz.Kill();
	return TRIERROR;
}

