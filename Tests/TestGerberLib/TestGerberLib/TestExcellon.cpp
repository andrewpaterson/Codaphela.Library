#include "BaseLib/LogToMemory.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "GerberLib/ExcellonParser.h"
#include "GerberLib/ExcellonInterpreter.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
class CTestExcellonListener : public CExcellonInterpreterListener
{
public:
	CChars	msz;
	size	muiNumHoles;

	void Init(void)
	{
		msz.Init();
		muiNumHoles = 0;
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

	void Hole(CExcellonCommandToolDefine* pcTool, SGerberPoint* psPoint)
	{
		muiNumHoles++;
		msz.Append("Hole T");
		msz.Append(pcTool->GetTool());
		AppendPoint(psPoint);
		msz.AppendNewLine();
	}

	void Slot(CExcellonCommandToolDefine* pcTool, SGerberPoint* psStart, SGerberPoint* psEnd)
	{
		msz.Append("Slot T");
		msz.Append(pcTool->GetTool());
		AppendPoint(psStart);
		AppendPoint(psEnd);
		msz.AppendNewLine();
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseExcellonText(char* szText, CExcellonCommands* pcCommands)
{
	CExcellonParser	cParser;
	TRISTATE		tResult;

	cParser.Init(szText, strlen(szText), "none.drl", pcCommands);
	tResult = cParser.Parse();
	cParser.Kill();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseExcellonFile(char* szName, CExcellonCommands* pcCommands)
{
	CTextFile		cFile;
	CExcellonParser	cParser;
	CChars			szFileName;
	TRISTATE		tResult;

	szFileName.Init("Input" _FS_ "KiCad" _FS_);
	szFileName.Append(szName);

	cFile.Init();
	if (!cFile.Read(szFileName.Text()))
	{
		cFile.Kill();
		szFileName.Kill();
		return TRIERROR;
	}

	//The Excellon parser accepts CRLF line endings; the KiCad files have them.
	cParser.Init(cFile.Text(), cFile.Length(), szFileName.Text(), pcCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	cFile.Kill();
	szFileName.Kill();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertExcellonParseError(char* szText, char* szExpectedError)
{
	CExcellonCommands	cCommands;
	TRISTATE			tResult;
	CLogToMemory		cMemoryLog;
	char				szLogText[1000];

	cMemoryLog.Start(true);
	cCommands.Init();
	tResult = ParseExcellonText(szText, &cCommands);
	cCommands.Kill();
	cMemoryLog.Stop(szLogText, 1000);

	AssertTrierror(tResult);
	AssertTrue(strstr(szLogText, szExpectedError) != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertExcellonInterpretError(char* szText, char* szExpectedError)
{
	CExcellonCommands		cCommands;
	CExcellonInterpreter	cInterpreter;
	CTestExcellonListener	cListener;
	CLogToMemory			cMemoryLog;
	char					szLogText[1000];
	bool					bResult;

	cCommands.Init();
	AssertTritrue(ParseExcellonText(szText, &cCommands));

	cListener.Init();
	cMemoryLog.Start(true);
	cInterpreter.Init(&cCommands, &cListener);
	bResult = cInterpreter.Interpret();
	cInterpreter.Kill();
	cMemoryLog.Stop(szLogText, 1000);
	cListener.Kill();
	cCommands.Kill();

	AssertFalse(bResult);
	AssertTrue(strstr(szLogText, szExpectedError) != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExcellonParser(void)
{
	CExcellonCommands			cCommands;
	TRISTATE					tResult;
	CExcellonCommandAttribute*	pcAttribute;
	CExcellonCommandToolDefine*	pcToolDefine;
	CExcellonCommandCoordinate*		pcHit;

	cCommands.Init();
	tResult = ParseExcellonText(
		"M48\r\n"
		"; DRILL file {KiCad 8.0.7} date 2025-01-13T07:42:03+0200\r\n"
		"; #@! TF.FileFunction,Plated,1,2,PTH\r\n"
		"FMAT,2\r\n"
		"METRIC\r\n"
		"; #@! TA.AperFunction,Plated,PTH,ViaDrill\r\n"
		"T1C0.4\r\n"
		"T12C1.\r\n"
		"%\r\n"
		"G90\r\n"
		"G05\r\n"
		"T1\r\n"
		"X1.5Y-2.25\r\n"
		"Y3.0\r\n"
		"\r\n"
		"T12\r\n"
		"X-.5Y+0.25  \r\n"
		"T0\r\n"
		"M30\r\n", &cCommands);

	AssertTritrue(tResult);
	AssertSize(18, cCommands.NumCommands());

	AssertTrue(cCommands.GetCommand(0)->IsType(EC_HeaderStart));
	AssertTrue(cCommands.GetCommand(1)->IsType(EC_Comment));
	AssertString("DRILL file {KiCad 8.0.7} date 2025-01-13T07:42:03+0200", ((CExcellonCommandComment*)cCommands.GetCommand(1))->Text());

	AssertTrue(cCommands.GetCommand(2)->IsType(EC_FileAttribute));
	pcAttribute = (CExcellonCommandAttribute*)cCommands.GetCommand(2);
	AssertString(".FileFunction", pcAttribute->NameText());
	AssertSize(4, pcAttribute->NumValues());
	AssertString("Plated", pcAttribute->ValueText(0));
	AssertString("1", pcAttribute->ValueText(1));
	AssertString("2", pcAttribute->ValueText(2));
	AssertString("PTH", pcAttribute->ValueText(3));

	AssertTrue(cCommands.GetCommand(3)->IsType(EC_Format));
	AssertInt(2, ((CExcellonCommandFormat*)cCommands.GetCommand(3))->GetFormat());
	AssertTrue(cCommands.GetCommand(4)->IsType(EC_Unit));
	AssertTrue(((CExcellonCommandUnit*)cCommands.GetCommand(4))->IsMillimeters());

	AssertTrue(cCommands.GetCommand(5)->IsType(EC_ApertureAttribute));
	pcAttribute = (CExcellonCommandAttribute*)cCommands.GetCommand(5);
	AssertString(".AperFunction", pcAttribute->NameText());
	AssertSize(3, pcAttribute->NumValues());
	AssertString("ViaDrill", pcAttribute->ValueText(2));

	pcToolDefine = (CExcellonCommandToolDefine*)cCommands.GetCommand(6);
	AssertTrue(pcToolDefine->IsType(EC_ToolDefine));
	AssertInt(1, pcToolDefine->GetTool());
	AssertNumber("0.4", pcToolDefine->GetDiameter());
	pcToolDefine = (CExcellonCommandToolDefine*)cCommands.GetCommand(7);
	AssertInt(12, pcToolDefine->GetTool());
	AssertNumber("1", pcToolDefine->GetDiameter());

	AssertTrue(cCommands.GetCommand(8)->IsType(EC_HeaderEnd));
	AssertTrue(cCommands.GetCommand(9)->IsType(EC_Absolute));
	AssertTrue(cCommands.GetCommand(10)->IsType(EC_DrillMode));
	AssertTrue(cCommands.GetCommand(11)->IsType(EC_ToolSelect));
	AssertInt(1, ((CExcellonCommandToolSelect*)cCommands.GetCommand(11))->GetTool());

	pcHit = (CExcellonCommandCoordinate*)cCommands.GetCommand(12);
	AssertTrue(pcHit->IsType(EC_Hit));
	AssertTrue(pcHit->HasX());
	AssertTrue(pcHit->HasY());
	AssertNumber("1.5", pcHit->GetX());
	AssertNumber("-2.25", pcHit->GetY());
	pcHit = (CExcellonCommandCoordinate*)cCommands.GetCommand(13);
	AssertFalse(pcHit->HasX());
	AssertTrue(pcHit->HasY());
	AssertNumber("3", pcHit->GetY());

	pcHit = (CExcellonCommandCoordinate*)cCommands.GetCommand(15);
	AssertNumber("-0.5", pcHit->GetX());
	AssertNumber("0.25", pcHit->GetY());
	AssertInt(0, ((CExcellonCommandToolSelect*)cCommands.GetCommand(16))->GetTool());
	AssertTrue(cCommands.GetCommand(17)->IsType(EC_EndOfProgram));
	cCommands.Kill();

	AssertExcellonParseError("FMAT,2\nM48\n", "Excellon file must start with M48");
	AssertExcellonParseError("M48\nINCH,LZ\n%\nM30\n", "Excellon zero suppression and number formats are not supported");
	AssertExcellonParseError("M48\nMETRIC,000.000\n%\nM30\n", "Excellon zero suppression and number formats are not supported");
	AssertExcellonParseError("M48\nVER,1\n%\nM30\n", "Unknown Excellon header command");
	AssertExcellonParseError("M48\nT0C1\n%\nM30\n", "Excellon tool 0 cannot be defined");
	AssertExcellonParseError("M48\nT1F00S00C1\n%\nM30\n", "Excellon tool diameter (C) expected");
	AssertExcellonParseError("M48\nT1C\n%\nM30\n", "Excellon tool diameter expected");
	AssertExcellonParseError("M48\nT1C-1\n%\nM30\n", "Excellon tool diameter expected");
	AssertExcellonParseError("M48\nT1C1.0.0\n%\nM30\n", "Unexpected characters after Excellon tool definition");
	AssertExcellonParseError("M48\n%\nG91\nM30\n", "Excellon incremental coordinates (G91) are not supported");
	AssertExcellonParseError("M48\n%\nG02X1.0Y2.0A0.5\nM30\n", "Excellon circular routing (G02, G03) is not supported");
	AssertExcellonParseError("M48\n%\nX1.0Y1.0G85X2.0Y2.0G85\nM30\n", "Unexpected characters after Excellon coordinates");
	AssertExcellonParseError("M48\n%\nX\nM30\n", "Excellon X coordinate with a decimal point expected");
	AssertExcellonParseError("M48\n%\nX1Y-2\nM30\n", "Excellon X coordinate with a decimal point expected");
	AssertExcellonParseError("M48\n%\nX1.0Y-2\nM30\n", "Excellon Y coordinate with a decimal point expected");
	AssertExcellonParseError("M48\n%\nX1.0Y1.0G85\nM30\n", "Excellon slot (G85) end coordinates expected");
	AssertExcellonParseError("M48\n%\nG00\nM30\n", "Excellon coordinates expected");
	AssertExcellonParseError("M48\n%\nG01Z1.0\nM30\n", "Excellon coordinates expected");
	AssertExcellonParseError("M48\n%\nT1A\nM30\n", "Unexpected characters after Excellon tool selection");
	AssertExcellonParseError("M48\n%\nM71\nM30\n", "Unknown Excellon command");
	AssertExcellonParseError("M48\n%\nM30\nT1\n", "Nothing may follow the end of program (M30)");
	AssertExcellonParseError("M48\n%\n", "End of program (M30) is missing.");
	AssertExcellonParseError("M48\nINCH\nX1.0Y1.0\n", "none.drl(3): Unknown Excellon header command, could not parse Command:\nX1.0Y1.0");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExcellonInterpreter(void)
{
	CExcellonCommands		cCommands;
	CExcellonInterpreter	cInterpreter;
	CTestExcellonListener	cListener;

	cCommands.Init();
	AssertTritrue(ParseExcellonText(
		"M48\n"
		"INCH\n"
		"T1C0.0157\n"
		"T2C0.0394\n"
		"%\n"
		"G90\n"
		"G05\n"
		"T1\n"
		"X0.05Y-2.55\n"
		"X0.07\n"
		"Y-1.12\n"
		"T2\n"
		"X0.6Y-2.7\n"
		"T0\n"
		"M30\n", &cCommands));
	cListener.Init();
	cInterpreter.Init(&cCommands, &cListener);
	AssertTrue(cInterpreter.Interpret());
	AssertTrue(cInterpreter.GetUnit() == EU_Inches);
	AssertSize(2, cInterpreter.NumTools());
	AssertNumber("0.0394", cInterpreter.GetTool(2)->GetDiameter());
	AssertNull(cInterpreter.GetTool(3));
	AssertString(
		"Hole T1 0.05,-2.55\n"
		"Hole T1 0.07,-2.55\n"
		"Hole T1 0.07,-1.12\n"
		"Hole T2 0.6,-2.7\n", cListener.msz.Text());
	cInterpreter.Kill();
	cListener.Kill();
	cCommands.Kill();

	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT2\nM30\n", "Excellon tool T2 is not defined.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\nT1C2\n%\nM30\n", "Excellon tool T1 is already defined.");
	AssertExcellonInterpretError("M48\nINCH\nMETRIC\n%\nM30\n", "Excellon unit (INCH or METRIC) can only be set once.");
	AssertExcellonInterpretError("M48\nT1C1\n%\nG90\nT1\nX1.0Y1.0\nM30\n", "Excellon unit (INCH or METRIC) must be set before the first hole.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nT1\nX1.0Y1.0\nM30\n", "Excellon absolute mode (G90) must be set before the first hole.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nX1.0Y1.0\nM30\n", "Excellon hole needs a tool.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT1\nT0\nX1.0Y1.0\nM30\n", "Excellon hole needs a tool.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT1\nY1.0\nM30\n", "Excellon X coordinate omitted but there is no previous hole.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExcellonSlots(void)
{
	CExcellonCommands		cCommands;
	CExcellonInterpreter	cInterpreter;
	CTestExcellonListener	cListener;
	CExcellonCommandCoordinate*	pcCoordinate;
	CExcellonCommandSlot*	pcSlot;

	//KiCad writes oval holes with route mode (the default) or as canned slots (G85).
	cCommands.Init();
	AssertTritrue(ParseExcellonText(
		"M48\n"
		"METRIC\n"
		"; #@! TA.AperFunction,Plated,PTH,ComponentDrill\n"
		"T1C0.800\n"
		"; #@! TD\n"
		"T2C1.000\n"
		"%\n"
		"G90\n"
		"G05\n"
		"T1\n"
		"X10.0Y-20.0\n"
		"T2\n"
		"G00X12.5Y-20.0\n"
		"M15\n"
		"G01X12.5Y-22.0\n"
		"M16\n"
		"G05\n"
		"G00X15.0Y-20.0\n"
		"M15\n"
		"G01X16.0Y-20.0\n"
		"X16.0Y-21.0\n"
		"Y-22.0\n"
		"M16\n"
		"X18.0\n"
		"G05\n"
		"X20.0Y-20.0G85X21.5Y-20.0\n"
		"Y-25.0G85Y-26.0\n"
		"X30.0Y-30.0\n"
		"M30\n", &cCommands));

	AssertTrue(cCommands.GetCommand(4)->IsType(EC_DeleteAttribute));
	AssertString("", ((CExcellonCommandAttribute*)cCommands.GetCommand(4))->NameText());

	pcCoordinate = (CExcellonCommandCoordinate*)cCommands.GetCommand(12);
	AssertTrue(pcCoordinate->IsType(EC_RouteMove));
	AssertNumber("12.5", pcCoordinate->GetX());
	AssertNumber("-20", pcCoordinate->GetY());
	AssertTrue(cCommands.GetCommand(13)->IsType(EC_ToolDown));
	pcCoordinate = (CExcellonCommandCoordinate*)cCommands.GetCommand(14);
	AssertTrue(pcCoordinate->IsType(EC_RouteLinear));
	AssertNumber("-22", pcCoordinate->GetY());
	AssertTrue(cCommands.GetCommand(15)->IsType(EC_ToolUp));
	AssertTrue(cCommands.GetCommand(16)->IsType(EC_DrillMode));

	pcSlot = (CExcellonCommandSlot*)cCommands.GetCommand(25);
	AssertTrue(pcSlot->IsType(EC_Slot));
	AssertTrue(pcSlot->HasStartX());
	AssertTrue(pcSlot->HasStartY());
	AssertNumber("20", pcSlot->GetStartX());
	AssertNumber("21.5", pcSlot->GetEndX());
	AssertNumber("-20", pcSlot->GetEndY());
	pcSlot = (CExcellonCommandSlot*)cCommands.GetCommand(26);
	AssertFalse(pcSlot->HasStartX());
	AssertTrue(pcSlot->HasStartY());
	AssertFalse(pcSlot->HasEndX());
	AssertTrue(pcSlot->HasEndY());

	cListener.Init();
	cInterpreter.Init(&cCommands, &cListener);
	AssertTrue(cInterpreter.Interpret());
	AssertString(
		"Hole T1 10,-20\n"
		"Slot T2 12.5,-20 12.5,-22\n"
		"Slot T2 15,-20 16,-20\n"
		"Slot T2 16,-20 16,-21\n"
		"Slot T2 16,-21 16,-22\n"
		"Slot T2 20,-20 21.5,-20\n"
		"Slot T2 21.5,-25 21.5,-26\n"
		"Hole T2 30,-30\n", cListener.msz.Text());
	AssertSize(2, cListener.muiNumHoles);
	cInterpreter.Kill();
	cListener.Kill();
	cCommands.Kill();

	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT1\nG01X1.0Y1.0\nM30\n", "Excellon linear route (G01) needs route mode (G00).");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT1\nX1.0Y1.0\nM15\nM30\n", "Excellon tool down (M15) needs route mode (G00).");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT1\nX1.0Y1.0\nM16\nM30\n", "Excellon tool up (M16) needs route mode (G00).");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT1\nG00X1.0Y1.0\nT0\nM15\nM30\n", "Excellon tool down (M15) needs a tool.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT1\nG00X1.0Y1.0\nM15\nG00X2.0Y1.0\nM30\n", "Excellon rapid move (G00) with the tool down.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT1\nG00X1.0Y1.0\nM15\nX2.0Y1.0\nM30\n", "Excellon rapid move (G00) with the tool down.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\nT2C2\n%\nG90\nT1\nG00X1.0Y1.0\nM15\nT2\nM30\n", "Excellon tool cannot change with the tool down.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT1\nG00X1.0Y1.0\nM15\nG01X2.0Y1.0\nG05\nM30\n", "Excellon drill mode (G05) with the tool down.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT1\nG00X1.0Y1.0\nM15\nG01X2.0Y1.0\nM30\n", "Excellon end of program (M30) with the tool down.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT1\nG00X1.0Y1.0\nX1.0Y1.0G85X2.0Y2.0\nM30\n", "Excellon canned slot (G85) needs drill mode.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nX1.0Y1.0G85X2.0Y2.0\nM30\n", "Excellon hole needs a tool.");
	AssertExcellonInterpretError("M48\nINCH\nT1C1\n%\nG90\nT1\nY1.0G85X2.0Y2.0\nM30\n", "Excellon X coordinate omitted but there is no previous hole.");
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExcellonKiCadAdder16Bit(void)
{
	CExcellonCommands			cCommands;
	CExcellonInterpreter		cInterpreter;
	CTestExcellonListener		cListener;
	CExcellonCommandAttribute*	pcAttribute;

	cCommands.Init();
	AssertTritrue(ParseExcellonFile("Adder 16bit" _FS_ "Adder 16bit-PTH.drl", &cCommands));
	AssertSize(157, cCommands.NumCommands());
	pcAttribute = (CExcellonCommandAttribute*)cCommands.GetCommand(5);
	AssertTrue(pcAttribute->IsType(EC_FileAttribute));
	AssertString(".FileFunction", pcAttribute->NameText());
	AssertString("PTH", pcAttribute->ValueText(3));

	cListener.Init();
	cInterpreter.Init(&cCommands, &cListener);
	AssertTrue(cInterpreter.Interpret());
	AssertTrue(cInterpreter.GetUnit() == EU_Inches);
	AssertSize(2, cInterpreter.NumTools());
	AssertNumber("0.0157", cInterpreter.GetTool(1)->GetDiameter());
	AssertNumber("0.0394", cInterpreter.GetTool(2)->GetDiameter());
	AssertSize(139, cListener.muiNumHoles);
	AssertTrue(strncmp("Hole T1 0.05,-2.55\n", cListener.msz.Text(), 19) == 0);
	AssertTrue(cListener.msz.EndsWith("Hole T2 0.6,-2.7\n"));
	cInterpreter.Kill();
	cListener.Kill();
	cCommands.Kill();

	cCommands.Init();
	AssertTritrue(ParseExcellonFile("Adder 16bit" _FS_ "Adder 16bit-NPTH.drl", &cCommands));
	AssertSize(12, cCommands.NumCommands());
	pcAttribute = (CExcellonCommandAttribute*)cCommands.GetCommand(5);
	AssertString("NonPlated", pcAttribute->ValueText(0));

	cListener.Init();
	cInterpreter.Init(&cCommands, &cListener);
	AssertTrue(cInterpreter.Interpret());
	AssertSize(0, cInterpreter.NumTools());
	AssertSize(0, cListener.muiNumHoles);
	cInterpreter.Kill();
	cListener.Kill();
	cCommands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestExcellon(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	TestExcellonParser();
	TestExcellonInterpreter();
	TestExcellonSlots();
	TestExcellonKiCadAdder16Bit();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

