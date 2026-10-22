#include "BaseLib/Logger.h"
#include "GerberWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberWriter::Init(void)
{
	mpsz = NULL;
	mbFormatSpecified = false;
	muiXWholes = 0;
	muiXDecimals = 0;
	muiYWholes = 0;
	muiYDecimals = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberWriter::Kill(void)
{
	mpsz = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberWriter::Write(CGerberCommands* pcCommands, CChars* psz)
{
	Begin(psz);
	return WriteCommands(pcCommands);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberWriter::Begin(CChars* psz)
{
	mpsz = psz;
	mbFormatSpecified = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberWriter::WriteCommands(CGerberCommands* pcCommands)
{
	size	i;
	size	uiNumCommands;

	uiNumCommands = pcCommands->NumCommands();
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
bool CGerberWriter::WriteCommand(CGerberCommand* pcCommand)
{
	CGerberCommandFormatSpecifier*	pcFormat;
	CGerberCommandLoadMirroring*	pcMirroring;
	CGerberCommandStepAndRepeat*	pcStepAndRepeat;
	CGerberCommandPlotMode*			pcPlotMode;
	char*							szText;

	if (pcCommand->IsComment())
	{
		szText = ((CGerberCommandComment*)pcCommand)->Text();
		mpsz->Append("G04");
		if (szText[0] != '\0')
		{
			mpsz->Append(' ');
			WriteString(szText, false);
		}
		mpsz->Append('*');
	}
	else if (pcCommand->IsMeasurementMode())
	{
		mpsz->Append(((CGerberCommandMeasurementMode*)pcCommand)->IsInches() ? "%MOIN*%" : "%MOMM*%");
	}
	else if (pcCommand->IsFormatSpecifier())
	{
		pcFormat = (CGerberCommandFormatSpecifier*)pcCommand;
		mbFormatSpecified = true;
		muiXWholes = pcFormat->GetXWholeNumbers();
		muiXDecimals = pcFormat->GetXDecimals();
		muiYWholes = pcFormat->GetYWholeNumbers();
		muiYDecimals = pcFormat->GetYDecimals();
		mpsz->Append("%FSLAX");
		mpsz->Append((uint32)muiXWholes);
		mpsz->Append((uint32)muiXDecimals);
		mpsz->Append('Y');
		mpsz->Append((uint32)muiYWholes);
		mpsz->Append((uint32)muiYDecimals);
		mpsz->Append("*%");
	}
	else if (pcCommand->IsApertureMacro())
	{
		return WriteApertureMacro((CGerberCommandApertureMacro*)pcCommand);
	}
	else if (pcCommand->IsApertureDefine())
	{
		return WriteApertureDefine((CGerberCommandApertureDefine*)pcCommand);
	}
	else if (pcCommand->IsSetAperture())
	{
		mpsz->Append('D');
		mpsz->Append(((CGerberCommandSetAperture*)pcCommand)->GetDCode());
		mpsz->Append('*');
	}
	else if (pcCommand->IsPlotMode())
	{
		pcPlotMode = (CGerberCommandPlotMode*)pcCommand;
		if (pcPlotMode->IsLinear())
		{
			mpsz->Append("G01*");
		}
		else if (pcPlotMode->IsClockwise())
		{
			mpsz->Append("G02*");
		}
		else
		{
			mpsz->Append("G03*");
		}
	}
	else if (pcCommand->IsQuadrantMode())
	{
		mpsz->Append(((CGerberCommandQuadrantMode*)pcCommand)->IsSingleQuadrant() ? "G74*" : "G75*");
	}
	else if (pcCommand->IsOperation())
	{
		return WriteOperation((CGerberCommandOperation*)pcCommand);
	}
	else if (pcCommand->IsLoadPolarity())
	{
		mpsz->Append(((CGerberCommandLoadPolarity*)pcCommand)->IsClear() ? "%LPC*%" : "%LPD*%");
	}
	else if (pcCommand->IsLoadMirroring())
	{
		pcMirroring = (CGerberCommandLoadMirroring*)pcCommand;
		if (pcMirroring->IsX())
		{
			mpsz->Append("%LMX*%");
		}
		else if (pcMirroring->IsY())
		{
			mpsz->Append("%LMY*%");
		}
		else if (pcMirroring->IsXY())
		{
			mpsz->Append("%LMXY*%");
		}
		else
		{
			mpsz->Append("%LMN*%");
		}
	}
	else if (pcCommand->IsLoadRotation())
	{
		mpsz->Append("%LR");
		WriteNumber(((CGerberCommandLoadRotation*)pcCommand)->GetRotation());
		mpsz->Append("*%");
	}
	else if (pcCommand->IsLoadScaling())
	{
		mpsz->Append("%LS");
		WriteNumber(((CGerberCommandLoadScaling*)pcCommand)->GetScaling());
		mpsz->Append("*%");
	}
	else if (pcCommand->IsRegion())
	{
		mpsz->Append("G36*");
		mpsz->AppendNewLine();
		if (!WriteCommands(((CGerberCommandRegion*)pcCommand)->GetCommands()))
		{
			return false;
		}
		mpsz->Append("G37*");
	}
	else if (pcCommand->IsApertureBlock())
	{
		mpsz->Append("%ABD");
		mpsz->Append(((CGerberCommandApertureBlock*)pcCommand)->GetDCode());
		mpsz->Append("*%");
		mpsz->AppendNewLine();
		if (!WriteCommands(((CGerberCommandApertureBlock*)pcCommand)->GetCommands()))
		{
			return false;
		}
		mpsz->Append("%AB*%");
	}
	else if (pcCommand->IsStepAndRepeat())
	{
		pcStepAndRepeat = (CGerberCommandStepAndRepeat*)pcCommand;
		mpsz->Append("%SRX");
		mpsz->Append(pcStepAndRepeat->GetXRepeats());
		mpsz->Append('Y');
		mpsz->Append(pcStepAndRepeat->GetYRepeats());
		mpsz->Append('I');
		WriteNumber(pcStepAndRepeat->GetXStep());
		mpsz->Append('J');
		WriteNumber(pcStepAndRepeat->GetYStep());
		mpsz->Append("*%");
		mpsz->AppendNewLine();
		if (!WriteCommands(pcStepAndRepeat->GetCommands()))
		{
			return false;
		}
		mpsz->Append("%SR*%");
	}
	else if (pcCommand->IsFileAttribute())
	{
		WriteAttribute("TF", (CGerberCommandAttribute*)pcCommand);
	}
	else if (pcCommand->IsApertureAttribute())
	{
		WriteAttribute("TA", (CGerberCommandAttribute*)pcCommand);
	}
	else if (pcCommand->IsObjectAttribute())
	{
		WriteAttribute("TO", (CGerberCommandAttribute*)pcCommand);
	}
	else if (pcCommand->IsDeleteAttribute())
	{
		WriteAttribute("TD", (CGerberCommandAttribute*)pcCommand);
	}
	else if (pcCommand->IsEndOfFile())
	{
		mpsz->Append("M02*");
	}
	else
	{
		return Error("Unknown Gerber command.");
	}

	mpsz->AppendNewLine();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberWriter::WriteApertureDefine(CGerberCommandApertureDefine* pcDefine)
{
	size	i;
	size	uiNumParameters;

	mpsz->Append("%ADD");
	mpsz->Append(pcDefine->GetDCode());
	mpsz->Append(pcDefine->TemplateText());
	uiNumParameters = pcDefine->NumParameters();
	for (i = 0; i < uiNumParameters; i++)
	{
		mpsz->Append((i == 0) ? ',' : 'X');
		WriteNumber(pcDefine->GetParameter(i));
	}
	mpsz->Append("*%");
	mpsz->AppendNewLine();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberWriter::WriteApertureMacro(CGerberCommandApertureMacro* pcMacro)
{
	size	i;
	size	uiNumPrimitives;

	//Each primitive goes on its own line; the % follows the * of the last one.
	mpsz->Append("%AM");
	mpsz->Append(pcMacro->NameText());
	mpsz->Append('*');

	uiNumPrimitives = pcMacro->NumPrimitives();
	if (uiNumPrimitives == 0)
	{
		return Error("Aperture macro has no primitives.");
	}
	for (i = 0; i < uiNumPrimitives; i++)
	{
		mpsz->AppendNewLine();
		if (!WritePrimitive(pcMacro->GetPrimitive(i)))
		{
			return false;
		}
	}
	mpsz->Append('%');
	mpsz->AppendNewLine();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberWriter::WriteExpressions(const char* szCode, CGerberExpression** apcExpressions, size uiNumExpressions)
{
	size	i;

	mpsz->Append(szCode);
	for (i = 0; i < uiNumExpressions; i++)
	{
		mpsz->Append(',');
		if (!apcExpressions[i]->Print(mpsz))
		{
			return Error("Could not write aperture macro expression.");
		}
	}
	mpsz->Append('*');
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberWriter::WritePrimitive(CGerberApertureMacro* pcPrimitive)
{
	CGerberExpression*							apcExpressions[7];
	CGerberApertureMacroCircle*					pcCircle;
	CGerberApertureMacroVectorLine*				pcVectorLine;
	CGerberApertureMacroCenterLine*				pcCenterLine;
	CGerberApertureMacroOutline*				pcOutline;
	CGerberApertureMacroPolygon*				pcPolygon;
	CGerberApertureMacroThermal*				pcThermal;
	CGerberApertureMacroVariableDefinition*		pcVariable;
	CGerberExpressionPosition*					pcPosition;
	size										i;
	size										uiNumPositions;

	if (pcPrimitive->IsComment())
	{
		mpsz->Append("0 ");
		WriteString(((CGerberApertureMacroComment*)pcPrimitive)->Text(), false);
		mpsz->Append('*');
		return true;
	}
	else if (pcPrimitive->IsCircle())
	{
		pcCircle = (CGerberApertureMacroCircle*)pcPrimitive;
		apcExpressions[0] = pcCircle->GetExposure();
		apcExpressions[1] = pcCircle->GetDiameter();
		apcExpressions[2] = pcCircle->GetCenterX();
		apcExpressions[3] = pcCircle->GetCenterY();
		apcExpressions[4] = pcCircle->GetRotation();

		//The circle rotation is optional.
		return WriteExpressions("1", apcExpressions, apcExpressions[4]->IsSet() ? 5 : 4);
	}
	else if (pcPrimitive->IsVectorLine())
	{
		pcVectorLine = (CGerberApertureMacroVectorLine*)pcPrimitive;
		apcExpressions[0] = pcVectorLine->GetExposure();
		apcExpressions[1] = pcVectorLine->GetWidth();
		apcExpressions[2] = pcVectorLine->GetStartX();
		apcExpressions[3] = pcVectorLine->GetStartY();
		apcExpressions[4] = pcVectorLine->GetEndX();
		apcExpressions[5] = pcVectorLine->GetEndY();
		apcExpressions[6] = pcVectorLine->GetRotation();
		return WriteExpressions("20", apcExpressions, 7);
	}
	else if (pcPrimitive->IsCenterLine())
	{
		pcCenterLine = (CGerberApertureMacroCenterLine*)pcPrimitive;
		apcExpressions[0] = pcCenterLine->GetExposure();
		apcExpressions[1] = pcCenterLine->GetWidth();
		apcExpressions[2] = pcCenterLine->GetHeight();
		apcExpressions[3] = pcCenterLine->GetCenterX();
		apcExpressions[4] = pcCenterLine->GetCenterY();
		apcExpressions[5] = pcCenterLine->GetRotation();
		return WriteExpressions("21", apcExpressions, 6);
	}
	else if (pcPrimitive->IsOutline())
	{
		pcOutline = (CGerberApertureMacroOutline*)pcPrimitive;
		uiNumPositions = pcOutline->NumPositions();
		if (uiNumPositions < 1)
		{
			return Error("Aperture macro outline has no points.");
		}

		//The number of vertices excludes the start point.
		mpsz->Append("4,");
		if (!pcOutline->GetExposure()->Print(mpsz))
		{
			return Error("Could not write aperture macro expression.");
		}
		mpsz->Append(',');
		mpsz->Append((uint32)(uiNumPositions - 1));
		for (i = 0; i < uiNumPositions; i++)
		{
			pcPosition = pcOutline->GetPosition(i);
			mpsz->Append(',');
			if (!pcPosition->GetX()->Print(mpsz))
			{
				return Error("Could not write aperture macro expression.");
			}
			mpsz->Append(',');
			if (!pcPosition->GetY()->Print(mpsz))
			{
				return Error("Could not write aperture macro expression.");
			}
		}
		mpsz->Append(',');
		if (!pcOutline->GetRotation()->Print(mpsz))
		{
			return Error("Could not write aperture macro expression.");
		}
		mpsz->Append('*');
		return true;
	}
	else if (pcPrimitive->IsPolygon())
	{
		pcPolygon = (CGerberApertureMacroPolygon*)pcPrimitive;
		apcExpressions[0] = pcPolygon->GetExposure();
		apcExpressions[1] = pcPolygon->GetNumberOfVertices();
		apcExpressions[2] = pcPolygon->GetCenterX();
		apcExpressions[3] = pcPolygon->GetCenterY();
		apcExpressions[4] = pcPolygon->GetDiameter();
		apcExpressions[5] = pcPolygon->GetRotation();
		return WriteExpressions("5", apcExpressions, 6);
	}
	else if (pcPrimitive->IsThermal())
	{
		pcThermal = (CGerberApertureMacroThermal*)pcPrimitive;
		apcExpressions[0] = pcThermal->GetCenterX();
		apcExpressions[1] = pcThermal->GetCenterY();
		apcExpressions[2] = pcThermal->GetOuterDiameter();
		apcExpressions[3] = pcThermal->GetInnerDiameter();
		apcExpressions[4] = pcThermal->GetGapThickness();
		apcExpressions[5] = pcThermal->GetRotation();
		return WriteExpressions("7", apcExpressions, 6);
	}
	else if (pcPrimitive->IsVariableDefinition())
	{
		pcVariable = (CGerberApertureMacroVariableDefinition*)pcPrimitive;
		mpsz->Append('$');
		mpsz->Append(pcVariable->GetVariable());
		mpsz->Append('=');
		if (!pcVariable->GetExpression()->Print(mpsz))
		{
			return Error("Could not write aperture macro expression.");
		}
		mpsz->Append('*');
		return true;
	}
	return Error("Unknown aperture macro primitive.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberWriter::WriteOperation(CGerberCommandOperation* pcOperation)
{
	if ((pcOperation->HasX() || pcOperation->HasY() || pcOperation->HasI() || pcOperation->HasJ()) && !mbFormatSpecified)
	{
		return Error("Coordinates cannot be written before the Format Specification (FS).");
	}

	//I and J use the X and Y formats.
	if (pcOperation->HasX() && !WriteCoordinate('X', pcOperation->GetX(), muiXWholes, muiXDecimals))
	{
		return false;
	}
	if (pcOperation->HasY() && !WriteCoordinate('Y', pcOperation->GetY(), muiYWholes, muiYDecimals))
	{
		return false;
	}
	if (pcOperation->HasI() && !WriteCoordinate('I', pcOperation->GetI(), muiXWholes, muiXDecimals))
	{
		return false;
	}
	if (pcOperation->HasJ() && !WriteCoordinate('J', pcOperation->GetJ(), muiYWholes, muiYDecimals))
	{
		return false;
	}

	if (pcOperation->IsPlot())
	{
		mpsz->Append("D01*");
	}
	else if (pcOperation->IsMove())
	{
		mpsz->Append("D02*");
	}
	else
	{
		mpsz->Append("D03*");
	}
	mpsz->AppendNewLine();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberWriter::WriteCoordinate(char cAxis, CNumber* pcValue, uint16 uiWholes, uint16 uiDecimals)
{
	CNumber		cInteger;
	CNumber		cTen;
	CNumber		cLimit;
	uint16		i;
	CChars		sz;

	//Leading zeros omitted; the value times 10 ^ decimals must be an integer with at most wholes + decimals digits.  4.2.2
	cInteger.Init(pcValue);
	cTen.Init(10);
	for (i = 0; i < uiDecimals; i++)
	{
		cInteger.Multiply(&cTen);
	}
	cLimit.Init(1);
	for (i = 0; i < uiWholes + uiDecimals; i++)
	{
		cLimit.Multiply(&cTen);
	}

	if (!cInteger.IsInteger())
	{
		sz.Init("Coordinate ");
		sz.Append(cAxis);
		pcValue->Print(&sz);
		sz.Append(" has more decimals than the Format Specification (FS) allows.");
		Error(sz.Text());
		sz.Kill();
		return false;
	}
	if (cInteger.IsNegative())
	{
		cInteger.Negate();
		if (!cInteger.LessThan(&cLimit))
		{
			return Error("Coordinate has more digits than the Format Specification (FS) allows.");
		}
		cInteger.Negate();
	}
	else if (!cInteger.LessThan(&cLimit))
	{
		return Error("Coordinate has more digits than the Format Specification (FS) allows.");
	}

	mpsz->Append(cAxis);
	if (cInteger.IsZero())
	{
		mpsz->Append('0');
	}
	else
	{
		cInteger.Print(mpsz);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberWriter::WriteAttribute(const char* szCode, CGerberCommandAttribute* pcAttribute)
{
	size	i;
	size	uiNumValues;

	mpsz->Append('%');
	mpsz->Append(szCode);
	mpsz->Append(pcAttribute->NameText());
	uiNumValues = pcAttribute->NumValues();
	for (i = 0; i < uiNumValues; i++)
	{
		mpsz->Append(',');
		WriteString(pcAttribute->ValueText(i), true);
	}
	mpsz->Append("*%");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberWriter::WriteNumber(CNumber* pcNumber)
{
	if (pcNumber->IsZero())
	{
		mpsz->Append('0');
	}
	else
	{
		pcNumber->Print(mpsz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberWriter::WriteString(const char* szString, bool bField)
{
	const unsigned char*	pc;
	uint32					uiCodePoint;
	size					uiLength;
	size					i;

	//Strings are UTF-8.  Reserved characters and every non ASCII character are escaped so the file stays ASCII.  3.4.3
	pc = (const unsigned char*)szString;
	while (*pc != 0)
	{
		if ((*pc == '%') || (*pc == '*') || (*pc == '\\') || (bField && (*pc == ',')))
		{
			WriteEscape(*pc);
			pc++;
		}
		else if (*pc < 0x80)
		{
			mpsz->Append((char)*pc);
			pc++;
		}
		else
		{
			if ((*pc & 0xE0) == 0xC0)
			{
				uiLength = 2;
				uiCodePoint = *pc & 0x1F;
			}
			else if ((*pc & 0xF0) == 0xE0)
			{
				uiLength = 3;
				uiCodePoint = *pc & 0x0F;
			}
			else if ((*pc & 0xF8) == 0xF0)
			{
				uiLength = 4;
				uiCodePoint = *pc & 0x07;
			}
			else
			{
				uiLength = 0;
				uiCodePoint = 0;
			}

			for (i = 1; i < uiLength; i++)
			{
				if ((pc[i] & 0xC0) != 0x80)
				{
					uiLength = 0;
					break;
				}
				uiCodePoint = (uiCodePoint << 6) | (pc[i] & 0x3F);
			}

			if (uiLength == 0)
			{
				//Not UTF-8; keep the byte as the code point of the same value.
				WriteEscape(*pc);
				pc++;
			}
			else
			{
				WriteEscape(uiCodePoint);
				pc += uiLength;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberWriter::WriteEscape(uint32 uiCodePoint)
{
	static const char	szHex[] = "0123456789ABCDEF";
	int					iDigits;
	int					i;

	if (uiCodePoint <= 0xFFFF)
	{
		mpsz->Append("\\u");
		iDigits = 4;
	}
	else
	{
		mpsz->Append("\\U");
		iDigits = 8;
	}
	for (i = iDigits - 1; i >= 0; i--)
	{
		mpsz->Append(szHex[(uiCodePoint >> (i * 4)) & 0xF]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberWriter::Error(const char* szError)
{
	gcLogger.Error2(__METHOD__, " ", szError, NULL);
	return false;
}

