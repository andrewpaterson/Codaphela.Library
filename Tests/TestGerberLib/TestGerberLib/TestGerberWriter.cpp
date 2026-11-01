#include "BaseLib/LogToMemory.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "GerberLib/GerberParser.h"
#include "GerberLib/GerberWriter.h"
#include "GerberLib/GerberInterpreter.h"
#include "GerberLib/ExcellonParser.h"
#include "GerberLib/ExcellonWriter.h"
#include "GerberLib/ExcellonInterpreter.h"
#include "TestLib/Assert.h"


TRISTATE ParseGerberText(char* szText, CGerberCommands* pcCommands);
TRISTATE ParseGerberFile(char* szDirectory, char* szName, CGerberCommands* pcCommands);
TRISTATE ParseExcellonText(char* szText, CExcellonCommands* pcCommands);
TRISTATE ParseExcellonFile(char* szName, CExcellonCommands* pcCommands);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
class CRecordGerberListener : public CGerberInterpreterListener
{
public:
	CChars	msz;

	void Init(void)
	{
		msz.Init();
	}

	void Kill(void)
	{
		msz.Kill();
	}

	void AppendPoint(SGerberPoint* psPoint)
	{
		msz.Append(" ");
		psPoint->cX.Print(&msz);
		msz.Append(",");
		psPoint->cY.Print(&msz);
	}

	void AppendState(CGerberGraphicsState* pcState)
	{
		msz.Append(" D");
		msz.Append(pcState->mpcCurrentAperture ? (int32)pcState->mpcCurrentAperture->GetDCode() : -1);
		msz.Append(pcState->mePolarity == GP_Clear ? " C" : " D");
	}

	void Move(CGerberGraphicsState* pcState, SGerberPoint* psPoint)
	{
		msz.Append("Move");
		AppendPoint(psPoint);
		msz.AppendNewLine();
	}

	void Draw(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd)
	{
		msz.Append("Draw");
		AppendState(pcState);
		AppendPoint(psStart);
		AppendPoint(psEnd);
		msz.AppendNewLine();
	}

	void Arc(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd, SGerberPoint* psCenter, bool bClockwise)
	{
		msz.Append(bClockwise ? "ArcCW" : "ArcCCW");
		AppendState(pcState);
		AppendPoint(psStart);
		AppendPoint(psEnd);
		AppendPoint(psCenter);
		msz.AppendNewLine();
	}

	void Flash(CGerberGraphicsState* pcState, SGerberPoint* psPoint)
	{
		msz.Append("Flash");
		AppendState(pcState);
		AppendPoint(psPoint);
		msz.AppendNewLine();
	}

	void BeginRegion(CGerberGraphicsState* pcState)
	{
		msz.Append("BeginRegion\n");
	}

	void EndRegion(CGerberGraphicsState* pcState)
	{
		msz.Append("EndRegion\n");
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
class CRecordExcellonListener : public CExcellonInterpreterListener
{
public:
	CChars	msz;
	bool	mbInches;

	void Init(bool bInches)
	{
		msz.Init();
		mbInches = bInches;
	}

	void Kill(void)
	{
		msz.Kill();
	}

	void AppendMillimetres(CNumber* pcNumber)
	{
		CNumber	cNumber;
		CNumber	cInchToMillimetres;

		cNumber.Init(pcNumber);
		if (mbInches)
		{
			cNumber.Multiply(cInchToMillimetres.Init("25.4"));
		}
		msz.Append(" ");
		cNumber.Print(&msz);
	}

	void Hole(CExcellonCommandToolDefine* pcTool, SGerberPoint* psPoint)
	{
		msz.Append("Hole");
		AppendMillimetres(pcTool->GetDiameter());
		AppendMillimetres(&psPoint->cX);
		AppendMillimetres(&psPoint->cY);
		msz.AppendNewLine();
	}

	void Slot(CExcellonCommandToolDefine* pcTool, SGerberPoint* psStart, SGerberPoint* psEnd)
	{
		msz.Append("Slot");
		AppendMillimetres(pcTool->GetDiameter());
		AppendMillimetres(&psStart->cX);
		AppendMillimetres(&psStart->cY);
		AppendMillimetres(&psEnd->cX);
		AppendMillimetres(&psEnd->cY);
		msz.AppendNewLine();
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool WriteGerber(CGerberCommands* pcCommands, CChars* psz)
{
	CGerberWriter	cWriter;
	bool			bResult;

	cWriter.Init();
	bResult = cWriter.Write(pcCommands, psz);
	cWriter.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertGerberWritten(char* szText, char* szExpected)
{
	CGerberCommands		cCommands;
	CChars				sz;

	cCommands.Init();
	AssertTritrue(ParseGerberText(szText, &cCommands));
	sz.Init();
	AssertTrue(WriteGerber(&cCommands, &sz));
	AssertString(szExpected, sz.Text());
	sz.Kill();
	cCommands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InterpretGerberCommands(CGerberCommands* pcCommands, CChars* pszEvents)
{
	CGerberInterpreter		cInterpreter;
	CRecordGerberListener	cListener;

	cListener.Init();
	cInterpreter.Init(pcCommands, &cListener);
	AssertTrue(cInterpreter.Interpret());
	cInterpreter.Kill();
	pszEvents->Append(&cListener.msz);
	cListener.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertGerberRoundTrip(char* szDirectory, char* szName)
{
	CGerberCommands		cOriginal;
	CGerberCommands		cWritten;
	CChars				szWritten;
	CChars				szRewritten;
	CChars				szOriginalEvents;
	CChars				szWrittenEvents;
	CGerberParser		cParser;

	//Written text parses back to commands that write the same text and interpret to the same operations.
	cOriginal.Init();
	AssertTritrue(ParseGerberFile(szDirectory, szName, &cOriginal));
	szWritten.Init();
	AssertTrue(WriteGerber(&cOriginal, &szWritten));

	cWritten.Init();
	cParser.Init(szWritten.Text(), szWritten.Length(), szName, &cWritten);
	AssertTritrue(cParser.Parse());
	cParser.Kill();
	szRewritten.Init();
	AssertTrue(WriteGerber(&cWritten, &szRewritten));
	AssertString(szWritten.Text(), szRewritten.Text());

	szOriginalEvents.Init();
	szWrittenEvents.Init();
	InterpretGerberCommands(&cOriginal, &szOriginalEvents);
	InterpretGerberCommands(&cWritten, &szWrittenEvents);
	AssertTrue(szOriginalEvents.Length() > 0);
	AssertString(szOriginalEvents.Text(), szWrittenEvents.Text());

	szWrittenEvents.Kill();
	szOriginalEvents.Kill();
	szRewritten.Kill();
	cWritten.Kill();
	szWritten.Kill();
	cOriginal.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberWriterCommands(void)
{
	AssertGerberWritten(
		"G04 Every command*\n"
		"%TF.FileFunction,Copper,L1,Top*%\n"
		"%FSLAX36Y36*%\n"
		"%MOMM*%\n"
		"%AMTHERMAL80*\n"
		"0 Thermal*\n"
		"7,0,0,0.800,0.550,0.125,45*%\n"
		"%TA.AperFunction,SMDPad,CuDef*%\n"
		"%ADD10C,0.1*%\n"
		"%TD.AperFunction*%\n"
		"%ADD11R,1.150000X0.600000*%\n"
		"%ADD12O,1X2X0.3*%\n"
		"%ADD13P,1.5X6X-45*%\n"
		"%ADD19THERMAL80*%\n"
		"%TD*%\n"
		"G75*\n"
		"G74*\n"
		"G75*\n"
		"%LPD*%\n"
		"%LMXY*%\n"
		"%LR-22.5*%\n"
		"%LS0.80*%\n"
		"%LMN*%\n"
		"%LR0*%\n"
		"%LS1*%\n"
		"D10*\n"
		"X0Y2500000D02*\n"
		"G01*\n"
		"X0Y0D01*\n"
		"Y-1D01*\n"
		"G03*\n"
		"X37500000Y10000000I2500000J-0D01*\n"
		"G02*\n"
		"X-1000000Y0I0J-1000000D01*\n"
		"G01*\n"
		"%TO.N,GND*%\n"
		"D19*\n"
		"X28750000Y28750000D03*\n"
		"D03*\n"
		"%TD*%\n"
		"%LPC*%\n"
		"G36*\n"
		"X5000000Y20000000D02*\n"
		"Y37500000D01*\n"
		"X5000000D01*\n"
		"G37*\n"
		"%LPD*%\n"
		"%ABD20*%\n"
		"D10*\n"
		"X1000000Y1000000D03*\n"
		"%AB*%\n"
		"%SRX3Y2I5.0J4.50*%\n"
		"D20*\n"
		"X0Y0D03*\n"
		"%SR*%\n"
		"M02*\n",

		"G04 Every command*\n"
		"%TF.FileFunction,Copper,L1,Top*%\n"
		"%FSLAX36Y36*%\n"
		"%MOMM*%\n"
		"%AMTHERMAL80*\n"
		"0 Thermal*\n"
		"7,0,0,0.8,0.55,0.125,45*%\n"
		"%TA.AperFunction,SMDPad,CuDef*%\n"
		"%ADD10C,0.1*%\n"
		"%TD.AperFunction*%\n"
		"%ADD11R,1.15X0.6*%\n"
		"%ADD12O,1X2X0.3*%\n"
		"%ADD13P,1.5X6X-45*%\n"
		"%ADD19THERMAL80*%\n"
		"%TD*%\n"
		"G75*\n"
		"G74*\n"
		"G75*\n"
		"%LPD*%\n"
		"%LMXY*%\n"
		"%LR-22.5*%\n"
		"%LS0.8*%\n"
		"%LMN*%\n"
		"%LR0*%\n"
		"%LS1*%\n"
		"D10*\n"
		"X0Y2500000D02*\n"
		"G01*\n"
		"X0Y0D01*\n"
		"Y-1D01*\n"
		"G03*\n"
		"X37500000Y10000000I2500000J0D01*\n"
		"G02*\n"
		"X-1000000Y0I0J-1000000D01*\n"
		"G01*\n"
		"%TO.N,GND*%\n"
		"D19*\n"
		"X28750000Y28750000D03*\n"
		"D03*\n"
		"%TD*%\n"
		"%LPC*%\n"
		"G36*\n"
		"X5000000Y20000000D02*\n"
		"Y37500000D01*\n"
		"X5000000D01*\n"
		"G37*\n"
		"%LPD*%\n"
		"%ABD20*%\n"
		"D10*\n"
		"X1000000Y1000000D03*\n"
		"%AB*%\n"
		"%SRX3Y2I5J4.5*%\n"
		"D20*\n"
		"X0Y0D03*\n"
		"%SR*%\n"
		"M02*\n");

	//Coordinates are written in the format of the file.
	AssertGerberWritten("%FSLAX24Y35*%\nX12345Y-1D02*\nX0Y99999999D03*\n", "%FSLAX24Y35*%\nX12345Y-1D02*\nX0Y99999999D03*\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberWriterMacroExpressions(void)
{
	//Only the parentheses needed for the same expression tree are written; the calculator drops redundant ones when parsing.
	//The Gerber calculator binds / tighter than x.
	AssertGerberWritten(
		"%AMEXPRESSIONS*\n"
		"1,1,$1+$1,$2,$3*\n"
		"$4=$1x0.75*\n"
		"$5=(-$1+3x$3)/2+$4*\n"
		"$6=-$3+$2/2*\n"
		"$7=0-$7*\n"
		"$8=$1-($2-$3)*\n"
		"$9=$1x($2x$3)*\n"
		"$10=(1+1)x2*\n"
		"$11=3/2x$3*\n"
		"$12=$1x$2/$3*\n"
		"$13=($1x$2)/$3*\n"
		"$14=((($1)))*\n"
		"$15=$1x-$2*\n"
		"$16=-(($1+$2))*\n"
		"$17=$1/$2/$3*\n"
		"1,0,$4,$2,$3,30*\n"
		"21,1,$1,$2-2x$3,-$4,-$5+$2,0*\n"
		"4,1,3,1,-1,1,1,2,1,1,-1,30*%\n",

		"%AMEXPRESSIONS*\n"
		"1,1,$1+$1,$2,$3*\n"
		"$4=$1x0.75*\n"
		"$5=(-$1+3x$3)/2+$4*\n"
		"$6=-$3+$2/2*\n"
		"$7=0-$7*\n"
		"$8=$1-($2-$3)*\n"
		"$9=$1x($2x$3)*\n"
		"$10=(1+1)x2*\n"
		"$11=3/2x$3*\n"
		"$12=$1x$2/$3*\n"
		"$13=($1x$2)/$3*\n"
		"$14=$1*\n"
		"$15=$1x(-$2)*\n"
		"$16=-($1+$2)*\n"
		"$17=$1/$2/$3*\n"
		"1,0,$4,$2,$3,30*\n"
		"21,1,$1,$2-2x$3,-$4,-$5+$2,0*\n"
		"4,1,3,1,-1,1,1,2,1,1,-1,30*%\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberWriterEscapes(void)
{
	CGerberCommands					cCommands;
	CGerberCommandObjectAttribute*	pcAttribute;

	//Escapes are decoded to UTF-8 when parsed.  3.4.3
	cCommands.Init();
	AssertTritrue(ParseGerberText("%TO.P,IC3,1,\\u03A32,a\\u002Cb,\\U0001F600,\\u00a9*%\n", &cCommands));
	pcAttribute = (CGerberCommandObjectAttribute*)cCommands.GetCommand(0);
	AssertSize(6, pcAttribute->NumValues());
	AssertString("IC3", pcAttribute->ValueText(0));
	AssertString("\xCE\xA3" "2", pcAttribute->ValueText(2));
	AssertString("a,b", pcAttribute->ValueText(3));
	AssertString("\xF0\x9F\x98\x80", pcAttribute->ValueText(4));
	AssertString("\xC2\xA9", pcAttribute->ValueText(5));
	cCommands.Kill();

	//Reserved characters and non ASCII characters are escaped when written.
	AssertGerberWritten(
		"%TO.P,IC3,1,\\u03A32,a\\u002Cb,\\U0001F600,\\u00a9*%\n",
		"%TO.P,IC3,1,\\u03A32,a\\u002Cb,\\U0001F600,\\u00A9*%\n");
	AssertGerberWritten(
		"G04 50\\u0025 \\u002A \\u005C, \xCE\xA3*\n",
		"G04 50\\u0025 \\u002A \\u005C, \\u03A3*\n");
	AssertGerberWritten(
		"%AMNOTE*0 \\u0025 \\u002A\\u005C*1,1,1,0,0*%\n",
		"%AMNOTE*\n0 \\u0025 \\u002A\\u005C*\n1,1,1,0,0*%\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberWriterErrors(void)
{
	CGerberCommands				cCommands;
	CGerberCommandOperation*	pcOperation;
	CNumber						cNumber;
	CChars						sz;
	CLogToMemory				cMemoryLog;
	char						szLogText[1000];

	//A value that does not fit the format is an error; nothing is rounded.
	cCommands.Init();
	cCommands.AddFormatSpecifier(2, 4, 2, 4);
	pcOperation = cCommands.AddOperation(GC_D03);
	pcOperation->SetX(cNumber.Init("1.00001"));
	sz.Init();
	cMemoryLog.Start(true);
	AssertFalse(WriteGerber(&cCommands, &sz));
	cMemoryLog.Stop(szLogText, 1000);
	AssertTrue(strstr(szLogText, "Coordinate X1.00001 has more decimals than the Format Specification (FS) allows.") != NULL);
	sz.Kill();
	cCommands.Kill();

	cCommands.Init();
	cCommands.AddFormatSpecifier(2, 4, 2, 4);
	pcOperation = cCommands.AddOperation(GC_D03);
	pcOperation->SetY(cNumber.Init("-100"));
	sz.Init();
	cMemoryLog.Start(true);
	AssertFalse(WriteGerber(&cCommands, &sz));
	cMemoryLog.Stop(szLogText, 1000);
	AssertTrue(strstr(szLogText, "Coordinate has more digits than the Format Specification (FS) allows.") != NULL);
	sz.Kill();
	cCommands.Kill();

	cCommands.Init();
	pcOperation = cCommands.AddOperation(GC_D02);
	pcOperation->SetX(cNumber.Init("1"));
	sz.Init();
	cMemoryLog.Start(true);
	AssertFalse(WriteGerber(&cCommands, &sz));
	cMemoryLog.Stop(szLogText, 1000);
	AssertTrue(strstr(szLogText, "Coordinates cannot be written before the Format Specification (FS).") != NULL);
	sz.Kill();
	cCommands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberWriterRoundTrip(void)
{
	AssertGerberRoundTrip("Ucamco", "2-13-1_Two_square_boxes.gbr");
	AssertGerberRoundTrip("Ucamco", "2-13-2_Polarities_and_Apertures.gbr");
	AssertGerberRoundTrip("Ucamco", "4-11-6_Block_with_different_orientations.gbr");
	AssertGerberRoundTrip("Ucamco", "4-6-4_Nested_blocks.gbr");
	AssertGerberRoundTrip("Ucamco", "6-1-6-2_A_drill_file.gbr");
	AssertGerberRoundTrip("Ucamco", "SMD_prim_20.gbr");
	AssertGerberRoundTrip("Ucamco", "SMD_prim_21_X1.gbr");
	AssertGerberRoundTrip("Ucamco", "sample_macro.gbr");
	AssertGerberRoundTrip("Ucamco", "sample_macro_X1.gbr");
	AssertGerberRoundTrip("KiCad" _FS_ "Adder 16bit", "Adder 16bit-F_Cu.gtl");
	AssertGerberRoundTrip("KiCad" _FS_ "Adder 16bit", "Adder 16bit-B_Cu.gbl");
	AssertGerberRoundTrip("KiCad" _FS_ "Adder 16bit", "Adder 16bit-F_Mask.gts");
	AssertGerberRoundTrip("KiCad" _FS_ "Adder 16bit", "Adder 16bit-F_Paste.gtp");
	AssertGerberRoundTrip("KiCad" _FS_ "Adder 16bit", "Adder 16bit-F_Silkscreen.gto");
	AssertGerberRoundTrip("KiCad" _FS_ "Adder 16bit", "Adder 16bit-Edge_Cuts.gm1");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool WriteExcellon(CExcellonCommands* pcCommands, CChars* psz)
{
	CExcellonWriter	cWriter;
	bool			bResult;

	cWriter.Init();
	bResult = cWriter.Write(pcCommands, psz);
	cWriter.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InterpretExcellonCommands(CExcellonCommands* pcCommands, CChars* pszEvents)
{
	CExcellonInterpreter		cInterpreter;
	CExcellonInterpreterListener	cUnits;
	CRecordExcellonListener		cListener;
	bool						bInches;

	cInterpreter.Init(pcCommands, &cUnits);
	AssertTrue(cInterpreter.Interpret());
	bInches = cInterpreter.GetUnit() == EU_Inches;
	cInterpreter.Kill();

	cListener.Init(bInches);
	cInterpreter.Init(pcCommands, &cListener);
	AssertTrue(cInterpreter.Interpret());
	cInterpreter.Kill();
	pszEvents->Append(&cListener.msz);
	cListener.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExcellonWriter(void)
{
	CExcellonCommands	cCommands;
	CExcellonCommands	cWritten;
	CChars				sz;
	CChars				szRewritten;
	CChars				szOriginalEvents;
	CChars				szWrittenEvents;

	//Inches are written as millimetres.
	cCommands.Init();
	AssertTritrue(ParseExcellonText(
		"M48\r\n"
		"; DRILL file {KiCad 8.0.7} date 2025-01-13T07:42:03+0200\r\n"
		"; FORMAT={-:-/ absolute / inch / decimal}\r\n"
		"; #@! TF.FileFunction,Plated,1,2,PTH\r\n"
		"FMAT,2\r\n"
		"INCH\r\n"
		"; #@! TA.AperFunction,Plated,PTH,ViaDrill\r\n"
		"T1C0.0157\r\n"
		"; #@! TD\r\n"
		"T2C0.0394\r\n"
		"%\r\n"
		"G90\r\n"
		"G05\r\n"
		"T1\r\n"
		"X0.05Y-2.55\r\n"
		"Y1.0\r\n"
		"T2\r\n"
		"G00X1.0Y-0.5\r\n"
		"M15\r\n"
		"G01X1.0Y-0.6\r\n"
		"Y-0.7\r\n"
		"M16\r\n"
		"G05\r\n"
		"X2.0Y-2.0G85X2.1Y-2.0\r\n"
		"T0\r\n"
		"M30\r\n", &cCommands));
	sz.Init();
	AssertTrue(WriteExcellon(&cCommands, &sz));
	AssertString(
		"M48\n"
		"; DRILL file {KiCad 8.0.7} date 2025-01-13T07:42:03+0200\n"
		"; FORMAT={-:-/ absolute / metric / decimal}\n"
		"; #@! TF.FileFunction,Plated,1,2,PTH\n"
		"FMAT,2\n"
		"METRIC\n"
		"; #@! TA.AperFunction,Plated,PTH,ViaDrill\n"
		"T1C0.39878\n"
		"; #@! TD\n"
		"T2C1.00076\n"
		"%\n"
		"G90\n"
		"G05\n"
		"T1\n"
		"X1.27Y-64.77\n"
		"Y25.4\n"
		"T2\n"
		"G00X25.4Y-12.7\n"
		"M15\n"
		"G01X25.4Y-15.24\n"
		"Y-17.78\n"
		"M16\n"
		"G05\n"
		"X50.8Y-50.8G85X53.34Y-50.8\n"
		"T0\n"
		"M30\n", sz.Text());

	//The millimetre file drills the same holes and slots.
	cWritten.Init();
	AssertTritrue(ParseExcellonText(sz.Text(), &cWritten));
	szOriginalEvents.Init();
	szWrittenEvents.Init();
	InterpretExcellonCommands(&cCommands, &szOriginalEvents);
	InterpretExcellonCommands(&cWritten, &szWrittenEvents);
	AssertString(
		"Hole 0.39878 1.27 -64.77\n"
		"Hole 0.39878 1.27 25.4\n"
		"Slot 1.00076 25.4 -12.7 25.4 -15.24\n"
		"Slot 1.00076 25.4 -15.24 25.4 -17.78\n"
		"Slot 1.00076 50.8 -50.8 53.34 -50.8\n", szOriginalEvents.Text());
	AssertString(szOriginalEvents.Text(), szWrittenEvents.Text());

	//Writing the millimetre file again changes nothing.
	szRewritten.Init();
	AssertTrue(WriteExcellon(&cWritten, &szRewritten));
	AssertString(sz.Text(), szRewritten.Text());

	szRewritten.Kill();
	szWrittenEvents.Kill();
	szOriginalEvents.Kill();
	cWritten.Kill();
	sz.Kill();
	cCommands.Kill();

	//Millimetres are written unchanged and coordinates always have a decimal point.
	cCommands.Init();
	AssertTritrue(ParseExcellonText("M48\nMETRIC\nT1C1\n%\nG90\nT1\nX1.0Y-2.50\nX0.0Y0.0\nM30\n", &cCommands));
	sz.Init();
	AssertTrue(WriteExcellon(&cCommands, &sz));
	AssertString("M48\nMETRIC\nT1C1.0\n%\nG90\nT1\nX1.0Y-2.5\nX0.0Y0.0\nM30\n", sz.Text());
	sz.Kill();
	cCommands.Kill();

	//The KiCad adder drill file in inches.
	cCommands.Init();
	AssertTritrue(ParseExcellonFile("Adder 16bit" _FS_ "Adder 16bit-PTH.drl", &cCommands));
	sz.Init();
	AssertTrue(WriteExcellon(&cCommands, &sz));
	AssertTrue(sz.Contains("\nMETRIC\n"));
	AssertFalse(sz.Contains("INCH"));
	AssertTrue(sz.Contains("\nT1C0.39878\n"));
	AssertTrue(sz.Contains("\nT2C1.00076\n"));
	cWritten.Init();
	AssertTritrue(ParseExcellonText(sz.Text(), &cWritten));
	szOriginalEvents.Init();
	szWrittenEvents.Init();
	InterpretExcellonCommands(&cCommands, &szOriginalEvents);
	InterpretExcellonCommands(&cWritten, &szWrittenEvents);
	AssertTrue(szOriginalEvents.StartsWith("Hole 0.39878 1.27 -64.77\n"));
	AssertString(szOriginalEvents.Text(), szWrittenEvents.Text());
	szWrittenEvents.Kill();
	szOriginalEvents.Kill();
	cWritten.Kill();
	sz.Kill();
	cCommands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberWriter(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	TestGerberWriterCommands();
	TestGerberWriterMacroExpressions();
	TestGerberWriterEscapes();
	TestGerberWriterErrors();
	TestGerberWriterRoundTrip();
	TestExcellonWriter();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

