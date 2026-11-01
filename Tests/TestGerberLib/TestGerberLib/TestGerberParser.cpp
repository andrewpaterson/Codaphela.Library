#include "SupportLib/Image.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NaiveFile.h"
#include "BaseLib/LogToMemory.h"
#include "GerberLib/GerberParser.h"
#include "GerberLib/GerberCommands.h"
#include "TestLib/Assert.h"


TRISTATE ParseGerberFile(char* szDirectory, char* szName, CGerberCommands* pcCommands);
#include "BaseLib/TextFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandEndOfFile(void)
{
	CChars					szGerberFile;
	CGerberParser			cParser;
	CGerberCommands			cCommands;
	TRISTATE				tResult;
	size					uiNumCommands;
	CGerberCommand*			pcCommand;

	szGerberFile.Init("M02*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(1, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsEndOfFile());

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandComment(void)
{
	CChars					szGerberFile;
	CGerberParser			cParser;
	CGerberCommands			cCommands;
	TRISTATE				tResult;
	size					uiNumCommands;
	CGerberCommand*			pcCommand;
	CGerberCommandComment*	pcComment;

	szGerberFile.Init(
		"G04 Ucamco ex. 1: Twon\n"
		"square boxes *\n"
		"G04 Ucamco ex. 2: Shapes*\n"
		"M02*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(3, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsComment());
	AssertTrue(pcCommand->IsType(GC_G04));

	pcComment = (CGerberCommandComment*)pcCommand;
	AssertString("Ucamco ex. 1: Twon\nsquare boxes ", pcComment->Text());

	pcCommand = cCommands.GetCommand(1);
	AssertTrue(pcCommand->IsComment());

	pcComment = (CGerberCommandComment*)pcCommand;
	AssertString("Ucamco ex. 2: Shapes", pcComment->Text());

	pcCommand = cCommands.GetCommand(2);
	AssertTrue(pcCommand->IsEndOfFile());

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandFormatSpecifier(void)
{
	CChars							szGerberFile;
	CGerberParser					cParser;
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	size							uiNumCommands;
	CGerberCommand*					pcCommand;
	CGerberCommandFormatSpecifier*	pcFormatSpecifier;
	uint16							ui;

	szGerberFile.Init(
		"%FSLAX26Y35*%\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(1, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsFormatSpecifier());
	AssertTrue(pcCommand->IsType(GC_FS));

	pcFormatSpecifier = (CGerberCommandFormatSpecifier*)pcCommand;

	ui = pcFormatSpecifier->GetXWholeNumbers();
	AssertShort((uint16)2, ui);

	ui = pcFormatSpecifier->GetXDecimals();
	AssertShort((uint16)6, ui);

	ui = pcFormatSpecifier->GetYWholeNumbers();
	AssertShort((uint16)3, ui);

	ui = pcFormatSpecifier->GetYDecimals();
	AssertShort((uint16)5, ui);

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandMeasurementMode(void)
{
	CChars							szGerberFile;
	CGerberParser					cParser;
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	size							uiNumCommands;
	CGerberCommand*					pcCommand;
	CGerberCommandMeasurementMode*	pcMeasurementMode;
	CLogToMemory					cMemoryLog;
	char							szLogText[200];
	char*							szSyntaxError;

	szGerberFile.Init("%MOMM*%");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(1, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsMeasurementMode());
	AssertTrue(pcCommand->IsType(GC_MO));

	pcMeasurementMode = (CGerberCommandMeasurementMode*)pcCommand;

	AssertTrue(pcMeasurementMode->IsMillimeters());
	AssertFalse(pcMeasurementMode->IsInches());

	cCommands.Kill();

	szGerberFile.Kill();


	szGerberFile.Init("%MOIN*%");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(1, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsMeasurementMode());

	pcMeasurementMode = (CGerberCommandMeasurementMode*)pcCommand;

	AssertTrue(pcMeasurementMode->IsInches());
	AssertFalse(pcMeasurementMode->IsMillimeters());

	cCommands.Kill();

	szGerberFile.Kill();


	cMemoryLog.Start(true);
	szGerberFile.Init("%MOPT*%");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTrierror(tResult);
	
	cMemoryLog.Stop(szLogText, 200);
	szSyntaxError = strstr(szLogText, "Syntax Error");
	AssertString(
"Syntax Error, could not parse Command:\n"
"%MOPT*\n"
"   ^\n", szSyntaxError);

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandPlotMode(void)
{
	CChars							szGerberFile;
	CGerberParser					cParser;
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	size							uiNumCommands;
	CGerberCommand*					pcCommand;
	CGerberCommandPlotMode*			pcPlotMode;

	szGerberFile.Init("G01*G02*G03*");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(3, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsPlotMode());
	AssertTrue(pcCommand->IsType(GC_G01));
	pcPlotMode = (CGerberCommandPlotMode*)pcCommand;
	AssertTrue(pcPlotMode->IsLinear());
	AssertFalse(pcPlotMode->IsCircular());

	pcCommand = cCommands.GetCommand(1);
	AssertTrue(pcCommand->IsPlotMode());
	AssertTrue(pcCommand->IsType(GC_G02));
	pcPlotMode = (CGerberCommandPlotMode*)pcCommand;
	AssertTrue(pcPlotMode->IsCircular());
	AssertFalse(pcPlotMode->IsLinear());
	AssertTrue(pcPlotMode->IsClockwise());
	AssertFalse(pcPlotMode->IsAnticlockwise());

	pcCommand = cCommands.GetCommand(2);
	AssertTrue(pcCommand->IsPlotMode());
	AssertTrue(pcCommand->IsType(GC_G03));
	pcPlotMode = (CGerberCommandPlotMode*)pcCommand;
	AssertTrue(pcPlotMode->IsCircular());
	AssertFalse(pcPlotMode->IsLinear());
	AssertFalse(pcPlotMode->IsClockwise());
	AssertTrue(pcPlotMode->IsAnticlockwise());

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandLoadPolarity(void)
{
	CChars							szGerberFile;
	CGerberParser					cParser;
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	size							uiNumCommands;
	CGerberCommand*					pcCommand;
	CGerberCommandLoadPolarity*		pcPolarity;

	szGerberFile.Init(
		"%LPC*%\n"
		"%LPD*%\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(2, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsLoadPolarity());
	AssertTrue(pcCommand->IsType(GC_LP));
	pcPolarity = (CGerberCommandLoadPolarity*)pcCommand;
	AssertTrue(pcPolarity->IsClear());
	AssertFalse(pcPolarity->IsDark());

	pcCommand = cCommands.GetCommand(1);
	AssertTrue(pcCommand->IsLoadPolarity());
	pcPolarity = (CGerberCommandLoadPolarity*)pcCommand;
	AssertTrue(pcPolarity->IsDark());
	AssertFalse(pcPolarity->IsClear());

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandLoadMirroring(void)
{
	CChars							szGerberFile;
	CGerberParser					cParser;
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	size							uiNumCommands;
	CGerberCommand*					pcCommand;
	CGerberCommandLoadMirroring*	pcMirroring;

	szGerberFile.Init(
		"%LMN*%\n"
		"%LMX*%\n"
		"%LMY*%\n"
		"%LMXY*%\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(4, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsLoadMirroring());
	AssertTrue(pcCommand->IsType(GC_LM));
	pcMirroring = (CGerberCommandLoadMirroring*)pcCommand;
	AssertTrue(pcMirroring->IsNone());

	pcCommand = cCommands.GetCommand(1);
	AssertTrue(pcCommand->IsLoadMirroring());
	pcMirroring = (CGerberCommandLoadMirroring*)pcCommand;
	AssertTrue(pcMirroring->IsX());

	pcCommand = cCommands.GetCommand(2);
	AssertTrue(pcCommand->IsLoadMirroring());
	pcMirroring = (CGerberCommandLoadMirroring*)pcCommand;
	AssertTrue(pcMirroring->IsY());

	pcCommand = cCommands.GetCommand(3);
	AssertTrue(pcCommand->IsLoadMirroring());
	pcMirroring = (CGerberCommandLoadMirroring*)pcCommand;
	AssertTrue(pcMirroring->IsXY());

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandLoadRotation(void)
{
	CChars							szGerberFile;
	CGerberParser					cParser;
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	size							uiNumCommands;
	CGerberCommand*					pcCommand;
	CGerberCommandLoadRotation*		pcRotation;

	szGerberFile.Init(
		"%LR90*%\n"
		"%LR0.008192*%\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(2, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsLoadRotation());
	AssertTrue(pcCommand->IsType(GC_LR));
	pcRotation = (CGerberCommandLoadRotation*)pcCommand;
	AssertNumber("90", pcRotation->GetRotation());

	pcCommand = cCommands.GetCommand(1);
	AssertTrue(pcCommand->IsLoadRotation());
	pcRotation = (CGerberCommandLoadRotation*)pcCommand;
	AssertNumber("0.008192", pcRotation->GetRotation());

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandLoadScaling(void)
{
	CChars							szGerberFile;
	CGerberParser					cParser;
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	size							uiNumCommands;
	CGerberCommand*					pcCommand;
	CGerberCommandLoadScaling*		pcScaling;

	szGerberFile.Init(
		"%LS90*%\n"
		"%LS0.008192*%\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(2, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsLoadScaling());
	AssertTrue(pcCommand->IsType(GC_LS));
	pcScaling = (CGerberCommandLoadScaling*)pcCommand;
	AssertNumber("90", pcScaling->GetScaling());

	pcCommand = cCommands.GetCommand(1);
	AssertTrue(pcCommand->IsLoadScaling());
	pcScaling = (CGerberCommandLoadScaling*)pcCommand;
	AssertNumber("0.008192", pcScaling->GetScaling());

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandFileAttribute(void)
{
	CChars							szGerberFile;
	CGerberParser					cParser;
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	size							uiNumCommands;
	CGerberCommand*					pcCommand;
	CGerberCommandFileAttribute*	pcAttribute;
	char*							szAttribute;
	char*							szValue;
	size							uiNumValues;

	szGerberFile.Init(
		"%TF.CreationDate,2016-04-25T00:00;00+01:00*%\n"
		"%TF.FileFunction,Soldermask,Top*%\n"
		"%TF.FileFunction,Copper,L1,Top*%");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(3, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsFileAttribute());
	AssertTrue(pcCommand->IsType(GC_TF));
	pcAttribute = (CGerberCommandFileAttribute*)pcCommand;

	szAttribute = pcAttribute->NameText();
	AssertString(".CreationDate", szAttribute);
	uiNumValues = pcAttribute->NumValues();
	AssertSize(1, uiNumValues);
	szValue = pcAttribute->ValueText(0);
	AssertString("2016-04-25T00:00;00+01:00", szValue);

	pcCommand = cCommands.GetCommand(1);
	AssertTrue(pcCommand->IsFileAttribute());
	pcAttribute = (CGerberCommandFileAttribute*)pcCommand;

	szAttribute = pcAttribute->NameText();
	AssertString(".FileFunction", szAttribute);
	uiNumValues = pcAttribute->NumValues();
	AssertSize(2, uiNumValues);
	szValue = pcAttribute->ValueText(0);
	AssertString("Soldermask", szValue);
	szValue = pcAttribute->ValueText(1);
	AssertString("Top", szValue);

	pcCommand = cCommands.GetCommand(2);
	AssertTrue(pcCommand->IsFileAttribute());
	pcAttribute = (CGerberCommandFileAttribute*)pcCommand;

	szAttribute = pcAttribute->NameText();
	AssertString(".FileFunction", szAttribute);
	uiNumValues = pcAttribute->NumValues();
	AssertSize(3, uiNumValues);
	szValue = pcAttribute->ValueText(0);
	AssertString("Copper", szValue);
	szValue = pcAttribute->ValueText(1);
	AssertString("L1", szValue);
	szValue = pcAttribute->ValueText(2);
	AssertString("Top", szValue);

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandApertureAttribute(void)
{
	CChars								szGerberFile;
	CGerberParser						cParser;
	CGerberCommands						cCommands;
	TRISTATE							tResult;
	size								uiNumCommands;
	CGerberCommand*						pcCommand;
	CGerberCommandApertureAttribute*	pcAttribute;
	char*								szAttribute;
	char*								szValue;
	size								uiNumValues;

	szGerberFile.Init("%TA.AperFunction,Conductor*%");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(1, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsApertureAttribute());
	AssertTrue(pcCommand->IsType(GC_TA));
	pcAttribute = (CGerberCommandApertureAttribute*)pcCommand;

	szAttribute = pcAttribute->NameText();
	AssertString(".AperFunction", szAttribute);
	uiNumValues = pcAttribute->NumValues();
	AssertSize(1, uiNumValues);
	szValue = pcAttribute->ValueText(0);
	AssertString("Conductor", szValue);

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandObjectAttribute(void)
{
	CChars								szGerberFile;
	CGerberParser						cParser;
	CGerberCommands						cCommands;
	TRISTATE							tResult;
	size								uiNumCommands;
	CGerberCommand*						pcCommand;
	CGerberCommandObjectAttribute*		pcAttribute;
	char*								szAttribute;
	char*								szValue;
	size								uiNumValues;

	szGerberFile.Init("%TO.C,R6*%");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(1, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsObjectAttribute());
	AssertTrue(pcCommand->IsType(GC_TO));
	pcAttribute = (CGerberCommandObjectAttribute*)pcCommand;

	szAttribute = pcAttribute->NameText();
	AssertString(".C", szAttribute);
	uiNumValues = pcAttribute->NumValues();
	AssertSize(1, uiNumValues);
	szValue = pcAttribute->ValueText(0);
	AssertString("R6", szValue);

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandDeleteAttribute(void)
{
	CChars								szGerberFile;
	CGerberParser						cParser;
	CGerberCommands						cCommands;
	TRISTATE							tResult;
	size								uiNumCommands;
	CGerberCommand*						pcCommand;
	CGerberCommandDeleteAttribute*		pcAttribute;

	szGerberFile.Init(
		"%TD.AperFunction*%\n"
		"%TD*%\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(2, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsDeleteAttribute());
	AssertTrue(pcCommand->IsType(GC_TD));
	pcAttribute = (CGerberCommandDeleteAttribute*)pcCommand;
	AssertString(".AperFunction", pcAttribute->NameText());
	AssertFalse(pcAttribute->IsDeleteAll());
	AssertSize(0, pcAttribute->NumValues());

	pcCommand = cCommands.GetCommand(1);
	AssertTrue(pcCommand->IsDeleteAttribute());
	pcAttribute = (CGerberCommandDeleteAttribute*)pcCommand;
	AssertString("", pcAttribute->NameText());
	AssertTrue(pcAttribute->IsDeleteAll());

	cCommands.Kill();

	szGerberFile.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandApertureDefine(void)
{
	CChars							szGerberFile;
	CGerberParser					cParser;
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	size							uiNumCommands;
	CGerberCommand*					pcCommand;
	CGerberCommandApertureDefine*	pcAperture;
	CLogToMemory					cMemoryLog;
	char							szLogText[400];

	szGerberFile.Init(
		"%ADD10C,0.5*%\n"
		"%ADD11R,1.150000X0.600000*%\n"
		"%ADD12O,1X2X0.3*%\n"
		"%ADD13P,1.5X6X-45*%\n"
		"%ADD100RoundRect,0.25X-0.475X0.25*%\n"
		"%ADD15Thermal*%\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(6, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsApertureDefine());
	AssertTrue(pcCommand->IsType(GC_AD));
	pcAperture = (CGerberCommandApertureDefine*)pcCommand;
	AssertInt(10, pcAperture->GetDCode());
	AssertString("C", pcAperture->TemplateText());
	AssertSize(1, pcAperture->NumParameters());
	AssertNumber("0.5", pcAperture->GetParameter(0));

	pcCommand = cCommands.GetCommand(1);
	AssertTrue(pcCommand->IsApertureDefine());
	pcAperture = (CGerberCommandApertureDefine*)pcCommand;
	AssertInt(11, pcAperture->GetDCode());
	AssertString("R", pcAperture->TemplateText());
	AssertSize(2, pcAperture->NumParameters());
	AssertNumber("1.15", pcAperture->GetParameter(0));
	AssertNumber("0.6", pcAperture->GetParameter(1));

	pcCommand = cCommands.GetCommand(2);
	pcAperture = (CGerberCommandApertureDefine*)pcCommand;
	AssertInt(12, pcAperture->GetDCode());
	AssertString("O", pcAperture->TemplateText());
	AssertSize(3, pcAperture->NumParameters());
	AssertNumber("1", pcAperture->GetParameter(0));
	AssertNumber("2", pcAperture->GetParameter(1));
	AssertNumber("0.3", pcAperture->GetParameter(2));

	pcCommand = cCommands.GetCommand(3);
	pcAperture = (CGerberCommandApertureDefine*)pcCommand;
	AssertInt(13, pcAperture->GetDCode());
	AssertString("P", pcAperture->TemplateText());
	AssertSize(3, pcAperture->NumParameters());
	AssertNumber("1.5", pcAperture->GetParameter(0));
	AssertNumber("6", pcAperture->GetParameter(1));
	AssertNumber("-45", pcAperture->GetParameter(2));

	pcCommand = cCommands.GetCommand(4);
	pcAperture = (CGerberCommandApertureDefine*)pcCommand;
	AssertInt(100, pcAperture->GetDCode());
	AssertString("RoundRect", pcAperture->TemplateText());
	AssertSize(3, pcAperture->NumParameters());
	AssertNumber("0.25", pcAperture->GetParameter(0));
	AssertNumber("-0.475", pcAperture->GetParameter(1));
	AssertNumber("0.25", pcAperture->GetParameter(2));

	pcCommand = cCommands.GetCommand(5);
	pcAperture = (CGerberCommandApertureDefine*)pcCommand;
	AssertInt(15, pcAperture->GetDCode());
	AssertString("Thermal", pcAperture->TemplateText());
	AssertSize(0, pcAperture->NumParameters());

	cCommands.Kill();

	szGerberFile.Kill();


	cMemoryLog.Start(true);
	szGerberFile.Init("%ADD9C,0.5*%");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTrierror(tResult);
	cMemoryLog.Stop(szLogText, 400);
	AssertTrue(strstr(szLogText, "Aperture D code must be 10 or greater") != NULL);

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandSetAperture(void)
{
	CChars						szGerberFile;
	CGerberParser				cParser;
	CGerberCommands				cCommands;
	TRISTATE					tResult;
	size						uiNumCommands;
	CGerberCommand*				pcCommand;
	CGerberCommandSetAperture*	pcSetAperture;

	szGerberFile.Init(
		"D10*\n"
		"D1234*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(2, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsSetAperture());
	AssertTrue(pcCommand->IsType(GC_Dnn));
	pcSetAperture = (CGerberCommandSetAperture*)pcCommand;
	AssertInt(10, pcSetAperture->GetDCode());

	pcCommand = cCommands.GetCommand(1);
	AssertTrue(pcCommand->IsSetAperture());
	pcSetAperture = (CGerberCommandSetAperture*)pcCommand;
	AssertInt(1234, pcSetAperture->GetDCode());

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandOperation(void)
{
	CChars						szGerberFile;
	CGerberParser				cParser;
	CGerberCommands				cCommands;
	TRISTATE					tResult;
	size						uiNumCommands;
	CGerberCommand*				pcCommand;
	CGerberCommandOperation*	pcOperation;
	CLogToMemory				cMemoryLog;
	char						szLogText[400];

	szGerberFile.Init(
		"%FSLAX26Y35*%\n"
		"X2500000Y-1000D02*\n"
		"X-1Y300000I1500000J-0D01*\n"
		"Y12345678D01*\n"
		"X0Y0D03*\n"
		"D03*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(6, uiNumCommands);

	pcCommand = cCommands.GetCommand(1);
	AssertTrue(pcCommand->IsOperation());
	AssertTrue(pcCommand->IsType(GC_D02));
	pcOperation = (CGerberCommandOperation*)pcCommand;
	AssertTrue(pcOperation->IsMove());
	AssertFalse(pcOperation->IsPlot());
	AssertFalse(pcOperation->IsFlash());
	AssertTrue(pcOperation->HasX());
	AssertTrue(pcOperation->HasY());
	AssertFalse(pcOperation->HasI());
	AssertFalse(pcOperation->HasJ());
	AssertNumber("2.5", pcOperation->GetX());
	AssertNumber("-0.01", pcOperation->GetY());

	pcCommand = cCommands.GetCommand(2);
	AssertTrue(pcCommand->IsType(GC_D01));
	pcOperation = (CGerberCommandOperation*)pcCommand;
	AssertTrue(pcOperation->IsPlot());
	AssertTrue(pcOperation->HasX());
	AssertTrue(pcOperation->HasY());
	AssertTrue(pcOperation->HasI());
	AssertTrue(pcOperation->HasJ());
	AssertNumber("-0.000001", pcOperation->GetX());
	AssertNumber("3", pcOperation->GetY());
	AssertNumber("1.5", pcOperation->GetI());
	AssertNumber("0", pcOperation->GetJ());

	pcCommand = cCommands.GetCommand(3);
	AssertTrue(pcCommand->IsType(GC_D01));
	pcOperation = (CGerberCommandOperation*)pcCommand;
	AssertFalse(pcOperation->HasX());
	AssertTrue(pcOperation->HasY());
	AssertNumber("123.45678", pcOperation->GetY());

	pcCommand = cCommands.GetCommand(4);
	AssertTrue(pcCommand->IsType(GC_D03));
	pcOperation = (CGerberCommandOperation*)pcCommand;
	AssertTrue(pcOperation->IsFlash());
	AssertNumber("0", pcOperation->GetX());
	AssertNumber("0", pcOperation->GetY());

	pcCommand = cCommands.GetCommand(5);
	AssertTrue(pcCommand->IsType(GC_D03));
	pcOperation = (CGerberCommandOperation*)pcCommand;
	AssertFalse(pcOperation->HasX());
	AssertFalse(pcOperation->HasY());

	cCommands.Kill();

	szGerberFile.Kill();


	cMemoryLog.Start(true);
	szGerberFile.Init("X100Y100D02*");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTrierror(tResult);
	cMemoryLog.Stop(szLogText, 400);
	AssertTrue(strstr(szLogText, "Coordinate before Format Specification (FS)") != NULL);
	AssertSize(0, cCommands.NumCommands());

	cCommands.Kill();

	szGerberFile.Kill();


	cMemoryLog.Start(true);
	szGerberFile.Init(
		"%FSLAX24Y24*%\n"
		"X1234567D02*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTrierror(tResult);
	cMemoryLog.Stop(szLogText, 400);
	AssertTrue(strstr(szLogText, "Coordinate has more digits than the Format Specification (FS) allows") != NULL);

	cCommands.Kill();

	szGerberFile.Kill();


	cMemoryLog.Start(true);
	szGerberFile.Init(
		"%FSLAX24Y24*%\n"
		"X10Y10I5J5D03*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTrierror(tResult);
	cMemoryLog.Stop(szLogText, 400);
	AssertTrue(strstr(szLogText, "I and J offsets are only allowed on D01") != NULL);

	cCommands.Kill();

	szGerberFile.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseGerberText(char* szText, CGerberCommands* pcCommands)
{
	CGerberParser	cParser;
	TRISTATE		tResult;

	cParser.Init(szText, strlen(szText), "none.txt", pcCommands);
	tResult = cParser.Parse();
	cParser.Kill();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseGerberFile(char* szName, CGerberCommands* pcCommands)
{
	return ParseGerberFile("Ucamco", szName, pcCommands);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE ParseGerberFile(char* szDirectory, char* szName, CGerberCommands* pcCommands)
{
	CTextFile		cFile;
	CGerberParser	cParser;
	CChars			szFileName;
	TRISTATE		tResult;

	szFileName.Init("Input" _FS_);
	szFileName.Append(szDirectory);
	szFileName.Append(_FS_);
	szFileName.Append(szName);

	cFile.Init();
	if (!cFile.Read(szFileName.Text()))
	{
		cFile.Kill();
		szFileName.Kill();
		return TRIERROR;
	}

	//The Ucamco and KiCad files have CRLF line endings.
	cFile.PassifyNewlines();

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
void AssertGerberError(char* szText, char* szExpectedError)
{
	CGerberCommands		cCommands;
	TRISTATE			tResult;
	CLogToMemory		cMemoryLog;
	char				szLogText[1000];

	cMemoryLog.Start(true);
	cCommands.Init();
	tResult = ParseGerberText(szText, &cCommands);
	cCommands.Kill();
	cMemoryLog.Stop(szLogText, 1000);

	AssertTrierror(tResult);
	AssertTrue(strstr(szLogText, szExpectedError) != NULL);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommandQuadrantMode(void)
{
	CGerberCommands				cCommands;
	TRISTATE					tResult;
	CGerberCommandQuadrantMode*	pcQuadrantMode;

	cCommands.Init();
	tResult = ParseGerberText(
		"G75*\n"
		"G74*\n", &cCommands);

	AssertTritrue(tResult);
	AssertSize(2, cCommands.NumCommands());

	AssertTrue(cCommands.GetCommand(0)->IsQuadrantMode());
	AssertTrue(cCommands.GetCommand(0)->IsType(GC_G75));
	pcQuadrantMode = (CGerberCommandQuadrantMode*)cCommands.GetCommand(0);
	AssertTrue(pcQuadrantMode->IsMultiQuadrant());
	AssertFalse(pcQuadrantMode->IsSingleQuadrant());

	AssertTrue(cCommands.GetCommand(1)->IsType(GC_G74));
	pcQuadrantMode = (CGerberCommandQuadrantMode*)cCommands.GetCommand(1);
	AssertTrue(pcQuadrantMode->IsSingleQuadrant());
	AssertFalse(pcQuadrantMode->IsMultiQuadrant());

	cCommands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserRegion(void)
{
	CGerberCommands			cCommands;
	TRISTATE				tResult;
	CGerberCommandRegion*	pcRegion;
	CGerberCommands*		pcContour;

	cCommands.Init();
	tResult = ParseGerberText(
		"%FSLAX26Y26*%\n"
		"G36*\n"
		"G04 First contour*\n"
		"X0Y0D02*\n"
		"G01*\n"
		"X1000000D01*\n"
		"Y1000000D01*\n"
		"G03*\n"
		"X0Y0I-500000J-500000D01*\n"
		"X2000000Y0D02*\n"
		"G01*\n"
		"X3000000D01*\n"
		"Y1000000D01*\n"
		"X2000000Y0D01*\n"
		"G37*\n"
		"X5000000Y5000000D03*\n", &cCommands);

	AssertTritrue(tResult);
	AssertSize(3, cCommands.NumCommands());

	AssertTrue(cCommands.GetCommand(1)->IsRegion());
	AssertTrue(cCommands.GetCommand(1)->IsCompound());
	AssertTrue(cCommands.GetCommand(1)->IsType(GC_G36));
	pcRegion = (CGerberCommandRegion*)cCommands.GetCommand(1);
	pcContour = pcRegion->GetCommands();
	AssertSize(12, pcContour->NumCommands());
	AssertTrue(pcContour->GetCommand(0)->IsComment());
	AssertTrue(pcContour->GetCommand(1)->IsType(GC_D02));
	AssertTrue(pcContour->GetCommand(2)->IsType(GC_G01));
	AssertTrue(pcContour->GetCommand(5)->IsType(GC_G03));
	AssertTrue(pcContour->GetCommand(6)->IsType(GC_D01));
	AssertTrue(((CGerberCommandOperation*)pcContour->GetCommand(6))->HasI());
	AssertTrue(pcContour->GetCommand(7)->IsType(GC_D02));
	AssertTrue(pcContour->GetCommand(11)->IsType(GC_D01));

	//Commands after the region go back into the file.
	AssertTrue(cCommands.GetCommand(2)->IsType(GC_D03));

	cCommands.Kill();

	AssertGerberError("%FSLAX26Y26*%\nG36*\nX0Y0D02*\nX5Y5D03*\nG37*\n", "D03 is not allowed in a region statement");
	AssertGerberError("%FSLAX26Y26*%\nG36*\nX0Y0D02*\nX5Y5D01*\n", "Region statement (G36) not closed with G37");
	AssertGerberError("%FSLAX26Y26*%\nG36*\nG01*\nX5Y5D01*\nG37*\n", "Region contour must start with a D02");
	AssertGerberError("G36*\nG37*\n", "Region statement (G36) must contain a contour");
	AssertGerberError("%FSLAX26Y26*%\nG36*\nX0Y0D02*\nD10*\nG37*\n", "Syntax Error");
	AssertGerberError("G37*\n", "Syntax Error");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserApertureBlock(void)
{
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	CGerberCommandApertureBlock*	pcBlock;
	CGerberCommands*				pcBlockCommands;

	cCommands.Init();
	tResult = ParseGerberText(
		"%FSLAX26Y26*%\n"
		"%ADD10C,1*%\n"
		"%ABD20*%\n"
		"D10*\n"
		"X0Y0D03*\n"
		"%ABD21*%\n"
		"%LPC*%\n"
		"X0Y0D03*\n"
		"G36*\n"
		"X0Y0D02*\n"
		"X1000000D01*\n"
		"Y1000000D01*\n"
		"X0Y0D01*\n"
		"G37*\n"
		"%AB*%\n"
		"D21*\n"
		"X1000000Y0D03*\n"
		"%AB*%\n"
		"D20*\n"
		"X0Y0D03*\n"
		"M02*\n", &cCommands);

	AssertTritrue(tResult);
	AssertSize(6, cCommands.NumCommands());

	AssertTrue(cCommands.GetCommand(2)->IsApertureBlock());
	AssertTrue(cCommands.GetCommand(2)->IsType(GC_AB));
	pcBlock = (CGerberCommandApertureBlock*)cCommands.GetCommand(2);
	AssertInt(20, pcBlock->GetDCode());
	pcBlockCommands = pcBlock->GetCommands();
	AssertSize(5, pcBlockCommands->NumCommands());
	AssertTrue(pcBlockCommands->GetCommand(0)->IsSetAperture());
	AssertTrue(pcBlockCommands->GetCommand(1)->IsType(GC_D03));
	AssertTrue(pcBlockCommands->GetCommand(3)->IsSetAperture());
	AssertTrue(pcBlockCommands->GetCommand(4)->IsType(GC_D03));

	AssertTrue(pcBlockCommands->GetCommand(2)->IsApertureBlock());
	pcBlock = (CGerberCommandApertureBlock*)pcBlockCommands->GetCommand(2);
	AssertInt(21, pcBlock->GetDCode());
	pcBlockCommands = pcBlock->GetCommands();
	AssertSize(3, pcBlockCommands->NumCommands());
	AssertTrue(pcBlockCommands->GetCommand(0)->IsLoadPolarity());
	AssertTrue(pcBlockCommands->GetCommand(1)->IsType(GC_D03));
	AssertTrue(pcBlockCommands->GetCommand(2)->IsRegion());
	AssertSize(4, ((CGerberCommandRegion*)pcBlockCommands->GetCommand(2))->GetCommands()->NumCommands());

	AssertTrue(cCommands.GetCommand(3)->IsSetAperture());
	AssertTrue(cCommands.GetCommand(4)->IsType(GC_D03));
	AssertTrue(cCommands.GetCommand(5)->IsEndOfFile());

	cCommands.Kill();

	AssertGerberError("%ABD10*%\nD11*\n", "Block aperture statement (AB) not closed");
	AssertGerberError("%ABD10*%\n%ABD11*%\n%AB*%\n", "Block aperture statement (AB) not closed");
	AssertGerberError("%ABD9*%\n%AB*%\n", "Aperture D code must be 10 or greater");
	AssertGerberError("%ABD10*%\n%MOMM*%\n%AB*%\n", "Syntax Error");
	AssertGerberError("%ABD10*%\n%SRX2Y2I1J1*%\n%SR*%\n%AB*%\n", "Syntax Error");
	AssertGerberError("%ABD10*%\nM02*\n", "Syntax Error");
	AssertGerberError("%AB*%\n", "Syntax Error");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserStepAndRepeat(void)
{
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	CGerberCommandStepAndRepeat*	pcStepAndRepeat;
	CGerberCommands*				pcRepeated;

	cCommands.Init();
	tResult = ParseGerberText(
		"%FSLAX26Y26*%\n"
		"%ADD10C,1*%\n"
		"%SRX3Y2I5.0J4.5*%\n"
		"D10*\n"
		"X0Y0D03*\n"
		"%ABD11*%\n"
		"X0Y0D03*\n"
		"%AB*%\n"
		"%SR*%\n"
		"%SRX1Y1I0J0*%\n"
		"%SR*%\n", &cCommands);

	AssertTritrue(tResult);
	AssertSize(4, cCommands.NumCommands());

	AssertTrue(cCommands.GetCommand(2)->IsStepAndRepeat());
	AssertTrue(cCommands.GetCommand(2)->IsType(GC_SR));
	pcStepAndRepeat = (CGerberCommandStepAndRepeat*)cCommands.GetCommand(2);
	AssertInt(3, pcStepAndRepeat->GetXRepeats());
	AssertInt(2, pcStepAndRepeat->GetYRepeats());
	AssertNumber("5", pcStepAndRepeat->GetXStep());
	AssertNumber("4.5", pcStepAndRepeat->GetYStep());
	pcRepeated = pcStepAndRepeat->GetCommands();
	AssertSize(3, pcRepeated->NumCommands());
	AssertTrue(pcRepeated->GetCommand(0)->IsSetAperture());
	AssertTrue(pcRepeated->GetCommand(1)->IsType(GC_D03));
	AssertTrue(pcRepeated->GetCommand(2)->IsApertureBlock());

	pcStepAndRepeat = (CGerberCommandStepAndRepeat*)cCommands.GetCommand(3);
	AssertTrue(pcStepAndRepeat->IsStepAndRepeat());
	AssertInt(1, pcStepAndRepeat->GetXRepeats());
	AssertInt(1, pcStepAndRepeat->GetYRepeats());
	AssertSize(0, pcStepAndRepeat->GetCommands()->NumCommands());

	cCommands.Kill();

	AssertGerberError("%SRX2Y2I1J1*%\n%SRX2Y2I1J1*%\n%SR*%\n%SR*%\n", "Syntax Error");
	AssertGerberError("%SRX2Y2I1J1*%\nG04 Not closed*\n", "Step and repeat statement (SR) not closed");
	AssertGerberError("%SRX0Y2I1J1*%\n%SR*%\n", "Step and repeat (SR) X and Y must be 1 or greater");
	AssertGerberError("%SRX2Y2I1*%\n%SR*%\n", "Syntax Error");
	AssertGerberError("%SR*%\n", "Syntax Error");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserVariableDefinitionMacro(void)
{
	CGerberCommands								cCommands;
	TRISTATE									tResult;
	CGerberCommandApertureMacro*				pcMacro;
	CGerberApertureMacroVariableDefinition*		pcVariable;

	cCommands.Init();
	tResult = ParseGerberText(
		"%AMDONUTCAL*\n"
		"1,1,$1,$2,$3*\n"
		"$4=$1x0.75*\n"
		"1,0,$4,$2,$3*\n"
		"$6=-$3+$2/2*\n"
		"$12=2.5*\n"
		"$7=(-$1+3x$3)/2+$4*%\n", &cCommands);

	AssertTritrue(tResult);
	AssertSize(1, cCommands.NumCommands());
	AssertTrue(cCommands.GetCommand(0)->IsApertureMacro());
	pcMacro = (CGerberCommandApertureMacro*)cCommands.GetCommand(0);
	AssertString("DONUTCAL", pcMacro->NameText());
	AssertSize(6, pcMacro->NumPrimitives());

	AssertTrue(pcMacro->GetPrimitive(0)->IsCircle());
	AssertFalse(pcMacro->GetPrimitive(0)->IsVariableDefinition());

	AssertTrue(pcMacro->GetPrimitive(1)->IsVariableDefinition());
	pcVariable = (CGerberApertureMacroVariableDefinition*)pcMacro->GetPrimitive(1);
	AssertInt(4, pcVariable->GetVariable());

	AssertTrue(pcMacro->GetPrimitive(2)->IsCircle());

	pcVariable = (CGerberApertureMacroVariableDefinition*)pcMacro->GetPrimitive(3);
	AssertTrue(pcVariable->IsVariableDefinition());
	AssertInt(6, pcVariable->GetVariable());

	pcVariable = (CGerberApertureMacroVariableDefinition*)pcMacro->GetPrimitive(4);
	AssertTrue(pcVariable->IsVariableDefinition());
	AssertInt(12, pcVariable->GetVariable());
	AssertDouble(2.5, pcVariable->GetExpression()->DoubleValue(), 2);

	pcVariable = (CGerberApertureMacroVariableDefinition*)pcMacro->GetPrimitive(5);
	AssertTrue(pcVariable->IsVariableDefinition());
	AssertInt(7, pcVariable->GetVariable());

	cCommands.Kill();

	AssertGerberError("%AMBAD*\n$0=1*%\n", "Macro variable must be $1 or greater");
	AssertGerberError("%AMBAD*\n$1 1*%\n", "Syntax Error");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserBadExpressionMacro(void)
{
	//Each of these fails in the calculator.  Run with LeakSanitizer to check the error paths free everything.
	AssertGerberError("%AMBAD*\n1,1,$1+,0,0*%\n", "Syntax Error");
	AssertGerberError("%AMBAD*\n1,1,+,0,0*%\n", "Syntax Error");
	AssertGerberError("%AMBAD*\n1,1,(1+2,0,0*%\n", "Syntax Error");
	AssertGerberError("%AMBAD*\n1,1,(),0,0*%\n", "Empty parentheses [()] in expression.");
	AssertGerberError("%AMBAD*\n1,1,1+(2x,0,0*%\n", "Syntax Error");
	AssertGerberError("%AMBAD*\n1,1,$1=,0,0*%\n", "Syntax Error");
	AssertGerberError("%AMBAD*\n1,1,1x/2,0,0*%\n", "Syntax Error");
	AssertGerberError("%AMBAD*\n1,1,2x,0,0*%\n", "Syntax Error");
	AssertGerberError("%AMBAD*\n1,1,x2,0,0*%\n", "Syntax Error");
	AssertGerberError("%AMBAD*\n1,1,((1),0,0*%\n", "Syntax Error");
	AssertGerberError("%AMBAD*\n1,1,1),0,0*%\n", "Syntax Error");
	AssertGerberError("%AMBAD*\n$1=*%\n", "Syntax Error");
	AssertGerberError("%AMBAD*\n$1=$2+*%\n", "Syntax Error");
	AssertGerberError("%AMBAD*\n$1=(3*%\n", "Syntax Error");
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCarriageReturns(void)
{
	CGerberParser		cParser;
	CGerberCommands		cCommands;
	CLogToMemory		cMemoryLog;
	char				szLogText[400];
	bool				bResult;

	//CTextParser does not accept carriage returns.  Text must be passified first.
	cMemoryLog.Start(true);
	cCommands.Init();
	bResult = cParser.Init("G04 Comment*\r\nM02*\r\n", 19, "none.txt", &cCommands);
	cParser.Kill();
	cCommands.Kill();
	cMemoryLog.Stop(szLogText, 400);

	AssertFalse(bResult);
	AssertTrue(strstr(szLogText, "carriage returns") != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserUcamcoExamples(void)
{
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	CGerberCommands*				pcInner;
	CGerberCommandApertureBlock*	pcBlock;
	CGerberCommandStepAndRepeat*	pcStepAndRepeat;
	CGerberCommandApertureMacro*	pcMacro;
	CGerberCommandQuadrantMode*		pcQuadrantMode;

	//Example 2.13.1 from the specification.
	cCommands.Init();
	tResult = ParseGerberFile("2-13-1_Two_square_boxes.gbr", &cCommands);
	AssertTritrue(tResult);
	AssertSize(19, cCommands.NumCommands());
	AssertTrue(cCommands.GetCommand(18)->IsEndOfFile());
	cCommands.Kill();

	//Example 2.13.2 from the specification.
	cCommands.Init();
	tResult = ParseGerberFile("2-13-2_Polarities_and_Apertures.gbr", &cCommands);
	AssertTritrue(tResult);
	AssertSize(61, cCommands.NumCommands());
	AssertTrue(cCommands.GetCommand(42)->IsQuadrantMode());
	pcQuadrantMode = (CGerberCommandQuadrantMode*)cCommands.GetCommand(42);
	AssertTrue(pcQuadrantMode->IsMultiQuadrant());
	AssertTrue(cCommands.GetCommand(47)->IsType(GC_D03));
	AssertTrue(cCommands.GetCommand(48)->IsRegion());
	pcInner = ((CGerberCommandRegion*)cCommands.GetCommand(48))->GetCommands();
	AssertSize(6, pcInner->NumCommands());
	AssertTrue(pcInner->GetCommand(0)->IsType(GC_D02));
	AssertTrue(pcInner->GetCommand(1)->IsType(GC_G01));
	AssertTrue(cCommands.GetCommand(49)->IsLoadPolarity());
	AssertTrue(cCommands.GetCommand(50)->IsRegion());
	pcInner = ((CGerberCommandRegion*)cCommands.GetCommand(50))->GetCommands();
	AssertSize(10, pcInner->NumCommands());
	AssertTrue(pcInner->GetCommand(2)->IsType(GC_G02));
	AssertTrue(cCommands.GetCommand(51)->IsLoadPolarity());
	AssertTrue(cCommands.GetCommand(60)->IsEndOfFile());
	cCommands.Kill();

	//Example 4.11.6 from the specification.
	cCommands.Init();
	tResult = ParseGerberFile("4-11-6_Block_with_different_orientations.gbr", &cCommands);
	AssertTritrue(tResult);
	AssertSize(26, cCommands.NumCommands());
	AssertTrue(cCommands.GetCommand(7)->IsComment());
	AssertTrue(cCommands.GetCommand(8)->IsApertureBlock());
	pcBlock = (CGerberCommandApertureBlock*)cCommands.GetCommand(8);
	AssertInt(12, pcBlock->GetDCode());
	pcInner = pcBlock->GetCommands();
	AssertSize(15, pcInner->NumCommands());
	AssertTrue(pcInner->GetCommand(0)->IsApertureDefine());
	AssertTrue(pcInner->GetCommand(11)->IsQuadrantMode());
	AssertTrue(pcInner->GetCommand(13)->IsType(GC_D01));
	AssertTrue(((CGerberCommandOperation*)pcInner->GetCommand(13))->HasI());
	AssertTrue(cCommands.GetCommand(9)->IsComment());
	cCommands.Kill();

	//Example 4.6.4 from the specification.
	cCommands.Init();
	tResult = ParseGerberFile("4-6-4_Nested_blocks.gbr", &cCommands);
	AssertTritrue(tResult);
	AssertSize(27, cCommands.NumCommands());
	pcBlock = (CGerberCommandApertureBlock*)cCommands.GetCommand(11);
	AssertTrue(pcBlock->IsApertureBlock());
	AssertInt(100, pcBlock->GetDCode());
	AssertSize(6, pcBlock->GetCommands()->NumCommands());
	pcBlock = (CGerberCommandApertureBlock*)cCommands.GetCommand(13);
	AssertTrue(pcBlock->IsApertureBlock());
	AssertInt(102, pcBlock->GetDCode());
	pcInner = pcBlock->GetCommands();
	AssertSize(11, pcInner->NumCommands());
	AssertTrue(pcInner->GetCommand(0)->IsComment());
	pcBlock = (CGerberCommandApertureBlock*)pcInner->GetCommand(1);
	AssertTrue(pcBlock->IsApertureBlock());
	AssertInt(101, pcBlock->GetDCode());
	AssertSize(5, pcBlock->GetCommands()->NumCommands());
	AssertTrue(pcInner->GetCommand(2)->IsSetAperture());
	AssertInt(101, ((CGerberCommandSetAperture*)pcInner->GetCommand(2))->GetDCode());
	cCommands.Kill();

	//Example 6.1.6.2 from the specification.
	cCommands.Init();
	tResult = ParseGerberFile("6-1-6-2_A_drill_file.gbr", &cCommands);
	AssertTritrue(tResult);
	AssertSize(51, cCommands.NumCommands());
	AssertTrue(cCommands.GetCommand(14)->IsDeleteAttribute());
	AssertTrue(cCommands.GetCommand(15)->IsDeleteAttribute());
	AssertString(".DrillTolerance", ((CGerberCommandDeleteAttribute*)cCommands.GetCommand(15))->NameText());
	cCommands.Kill();

	//Ucamco sample files.
	cCommands.Init();
	tResult = ParseGerberFile("SMD_prim_20.gbr", &cCommands);
	AssertTritrue(tResult);
	AssertSize(40, cCommands.NumCommands());
	AssertTrue(cCommands.GetCommand(10)->IsApertureMacro());
	pcMacro = (CGerberCommandApertureMacro*)cCommands.GetCommand(10);
	AssertString("RECTROUNDCORNERS", pcMacro->NameText());
	AssertSize(15, pcMacro->NumPrimitives());
	AssertTrue(pcMacro->GetPrimitive(9)->IsVectorLine());
	cCommands.Kill();

	cCommands.Init();
	tResult = ParseGerberFile("SMD_prim_20_X1.gbr", &cCommands);
	AssertTritrue(tResult);
	AssertSize(35, cCommands.NumCommands());
	cCommands.Kill();

	cCommands.Init();
	tResult = ParseGerberFile("SMD_prim_21.gbr", &cCommands);
	AssertTritrue(tResult);
	AssertSize(40, cCommands.NumCommands());
	pcMacro = (CGerberCommandApertureMacro*)cCommands.GetCommand(10);
	AssertTrue(pcMacro->IsApertureMacro());
	AssertSize(15, pcMacro->NumPrimitives());
	cCommands.Kill();

	cCommands.Init();
	tResult = ParseGerberFile("SMD_prim_21_X1.gbr", &cCommands);
	AssertTritrue(tResult);
	AssertSize(35, cCommands.NumCommands());
	cCommands.Kill();

	cCommands.Init();
	tResult = ParseGerberFile("sample_macro.gbr", &cCommands);
	AssertTritrue(tResult);
	AssertSize(57, cCommands.NumCommands());
	pcMacro = (CGerberCommandApertureMacro*)cCommands.GetCommand(8);
	AssertTrue(pcMacro->IsApertureMacro());
	AssertString("BOXR", pcMacro->NameText());
	AssertSize(17, pcMacro->NumPrimitives());
	AssertTrue(pcMacro->GetPrimitive(5)->IsComment());
	AssertTrue(pcMacro->GetPrimitive(6)->IsVariableDefinition());
	AssertInt(5, ((CGerberApertureMacroVariableDefinition*)pcMacro->GetPrimitive(6))->GetVariable());
	AssertInt(6, ((CGerberApertureMacroVariableDefinition*)pcMacro->GetPrimitive(7))->GetVariable());
	AssertInt(7, ((CGerberApertureMacroVariableDefinition*)pcMacro->GetPrimitive(8))->GetVariable());
	AssertTrue(pcMacro->GetPrimitive(10)->IsCenterLine());
	AssertTrue(pcMacro->GetPrimitive(16)->IsCircle());

	AssertTrue(cCommands.GetCommand(49)->IsRegion());
	AssertSize(5, ((CGerberCommandRegion*)cCommands.GetCommand(49))->GetCommands()->NumCommands());

	pcStepAndRepeat = (CGerberCommandStepAndRepeat*)cCommands.GetCommand(50);
	AssertTrue(pcStepAndRepeat->IsStepAndRepeat());
	AssertInt(1, pcStepAndRepeat->GetXRepeats());
	AssertInt(90, pcStepAndRepeat->GetYRepeats());
	AssertNumber("0", pcStepAndRepeat->GetXStep());
	AssertNumber("0.03", pcStepAndRepeat->GetYStep());
	AssertSize(5, pcStepAndRepeat->GetCommands()->NumCommands());

	pcStepAndRepeat = (CGerberCommandStepAndRepeat*)cCommands.GetCommand(51);
	AssertTrue(pcStepAndRepeat->IsStepAndRepeat());
	AssertInt(54, pcStepAndRepeat->GetXRepeats());
	AssertInt(1, pcStepAndRepeat->GetYRepeats());
	AssertNumber("0.03", pcStepAndRepeat->GetXStep());
	AssertNumber("0", pcStepAndRepeat->GetYStep());
	AssertSize(4, pcStepAndRepeat->GetCommands()->NumCommands());

	AssertTrue(cCommands.GetCommand(52)->IsComment());

	pcStepAndRepeat = (CGerberCommandStepAndRepeat*)cCommands.GetCommand(53);
	AssertTrue(pcStepAndRepeat->IsStepAndRepeat());
	AssertInt(1, pcStepAndRepeat->GetXRepeats());
	AssertInt(2, pcStepAndRepeat->GetYRepeats());
	AssertNumber("3", pcStepAndRepeat->GetYStep());
	AssertSize(114, pcStepAndRepeat->GetCommands()->NumCommands());

	AssertTrue(cCommands.GetCommand(54)->IsLoadPolarity());
	AssertTrue(cCommands.GetCommand(55)->IsFileAttribute());
	AssertTrue(cCommands.GetCommand(56)->IsEndOfFile());
	cCommands.Kill();

	cCommands.Init();
	tResult = ParseGerberFile("sample_macro_X1.gbr", &cCommands);
	AssertTritrue(tResult);
	AssertSize(52, cCommands.NumCommands());
	AssertTrue(cCommands.GetCommand(45)->IsRegion());
	AssertTrue(cCommands.GetCommand(46)->IsStepAndRepeat());
	AssertTrue(cCommands.GetCommand(47)->IsStepAndRepeat());
	AssertTrue(cCommands.GetCommand(49)->IsStepAndRepeat());
	AssertSize(114, ((CGerberCommandStepAndRepeat*)cCommands.GetCommand(49))->GetCommands()->NumCommands());
	cCommands.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCircleMacro(void)
{
	CChars							szGerberFile;
	CGerberParser					cParser;
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	size							uiNumCommands;
	CGerberCommand*					pcCommand;
	CGerberCommandApertureMacro*	pcApetureMacro;
	size							uiNumPrimitives;
	CGerberApertureMacro*			pcPrimitive;
	CGerberApertureMacroCircle*		pcCircle;
	CNumber*						pcNumber;

	szGerberFile.Init(
		"%AMCircle*\n"
		"1,1,1.5,4.36,2,90.8234795*%\n"
		"M02*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(2, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsApertureMacro());

	pcApetureMacro = (CGerberCommandApertureMacro*)pcCommand;
	uiNumPrimitives = pcApetureMacro->NumPrimitives();
	AssertSize(1, uiNumPrimitives);

	pcPrimitive = pcApetureMacro->GetPrimitive(0);
	AssertTrue(pcPrimitive->IsCircle());

	pcCircle = (CGerberApertureMacroCircle*)pcPrimitive;
	pcNumber = pcCircle->GetCenterX()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(4.36f, pcNumber->FloatValue(), 2);

	pcNumber = pcCircle->GetCenterY()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(2.0f, pcNumber->FloatValue(), 2);

	pcNumber = pcCircle->GetDiameter()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(1.5f, pcNumber->FloatValue(), 2);

	pcNumber = pcCircle->GetRotation()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(90.82348f, pcNumber->FloatValue(), 5);

	pcNumber = pcCircle->GetExposure()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(1.f, pcNumber->FloatValue(), 5);

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserVectorLineMacro(void)
{
	CChars								szGerberFile;
	CGerberParser						cParser;
	CGerberCommands						cCommands;
	TRISTATE							tResult;
	size								uiNumCommands;
	CGerberCommand*						pcCommand;
	CGerberCommandApertureMacro*		pcApetureMacro;
	size								uiNumPrimitives;
	CGerberApertureMacro*				pcPrimitive;
	CGerberApertureMacroVectorLine*		pcLine;
	Float64								pcValue;

	szGerberFile.Init(
		"%AMLine*\n"
		"20, 0, 2.3, 3, 4, 5, 6, 0*%\n"
		"M02*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(2, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsApertureMacro());

	pcApetureMacro = (CGerberCommandApertureMacro*)pcCommand;
	uiNumPrimitives = pcApetureMacro->NumPrimitives();
	AssertSize(1, uiNumPrimitives);

	pcPrimitive = pcApetureMacro->GetPrimitive(0);
	AssertTrue(pcPrimitive->IsVectorLine());
	pcLine = (CGerberApertureMacroVectorLine*)pcPrimitive;

	pcValue = pcLine->GetExposure()->DoubleValue();
	AssertDouble(0.0, pcValue, 2);

	pcValue = pcLine->GetWidth()->DoubleValue();
	AssertDouble(2.3, pcValue, 2);

	pcValue = pcLine->GetStartX()->DoubleValue();
	AssertDouble(3.0, pcValue, 2);

	pcValue = pcLine->GetStartY()->DoubleValue();
	AssertDouble(4.0, pcValue, 2);

	pcValue = pcLine->GetEndX()->DoubleValue();
	AssertDouble(5.0, pcValue, 2);

	pcValue = pcLine->GetEndY()->DoubleValue();
	AssertDouble(6.0, pcValue, 2);

	pcValue = pcLine->GetRotation()->DoubleValue();
	AssertDouble(0.0, pcValue, 2);

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCenterLineMacro(void)
{
	CChars								szGerberFile;
	CGerberParser						cParser;
	CGerberCommands						cCommands;
	TRISTATE							tResult;
	size								uiNumCommands;
	CGerberCommand*						pcCommand;
	CGerberCommandApertureMacro*		pcApetureMacro;
	size								uiNumPrimitives;
	CGerberApertureMacro*				pcPrimitive;
	CGerberApertureMacroCenterLine*		pcLine;
	Float64								pcValue;

	szGerberFile.Init(
		"%AMLine*\n"
		"21, 1, 2.3, 3.5, 3, 4, 87*%\n"
		"M02*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(2, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsApertureMacro());
	AssertTrue(pcCommand->IsType(GC_AM));

	pcApetureMacro = (CGerberCommandApertureMacro*)pcCommand;
	uiNumPrimitives = pcApetureMacro->NumPrimitives();
	AssertSize(1, uiNumPrimitives);

	pcPrimitive = pcApetureMacro->GetPrimitive(0);
	AssertTrue(pcPrimitive->IsCenterLine());
	pcLine = (CGerberApertureMacroCenterLine*)pcPrimitive;

	pcValue = pcLine->GetExposure()->DoubleValue();
	AssertDouble(1.0, pcValue, 2);

	pcValue = pcLine->GetWidth()->DoubleValue();
	AssertDouble(2.3, pcValue, 2);

	pcValue = pcLine->GetHeight()->DoubleValue();
	AssertDouble(3.5, pcValue, 2);

	pcValue = pcLine->GetCenterX()->DoubleValue();
	AssertDouble(3.0, pcValue, 2);

	pcValue = pcLine->GetCenterY()->DoubleValue();
	AssertDouble(4.0, pcValue, 2);

	pcValue = pcLine->GetRotation()->DoubleValue();
	AssertDouble(87.0, pcValue, 2);

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserOutlineMacro(void)
{
	CChars								szGerberFile;
	CGerberParser						cParser;
	CGerberCommands						cCommands;
	TRISTATE							tResult;
	size								uiNumCommands;
	CGerberCommand*						pcCommand;
	CGerberCommandApertureMacro*		pcApetureMacro;
	size								uiNumPrimitives;
	CGerberApertureMacro*				pcPrimitive;
	CGerberApertureMacroOutline*		pcShape;
	Float64								pcValue;
	size								uiNumPositions;
	CGerberExpressionPosition*			pcPosition;

	szGerberFile.Init(
		"%AMLine*\n"
		"4, 1, 6, 0,1, 2,3, 4,5, 6,7, 8,9, 10,11, 12,13, 45.235456*%\n"
		"M02*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(2, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsApertureMacro());

	pcApetureMacro = (CGerberCommandApertureMacro*)pcCommand;
	uiNumPrimitives = pcApetureMacro->NumPrimitives();
	AssertSize(1, uiNumPrimitives);

	pcPrimitive = pcApetureMacro->GetPrimitive(0);
	AssertTrue(pcPrimitive->IsOutline());
	pcShape = (CGerberApertureMacroOutline*)pcPrimitive;

	pcValue = pcShape->GetExposure()->DoubleValue();
	AssertDouble(1.0, pcValue, 2);

	uiNumPositions = pcShape->NumPositions();
	AssertSize(7, uiNumPositions);

	pcPosition = pcShape->GetPosition(0);
	pcValue = pcPosition->GetX()->DoubleValue();
	AssertDouble(0.0, pcValue, 2);
	pcValue = pcPosition->GetY()->DoubleValue();
	AssertDouble(1.0, pcValue, 2);

	pcPosition = pcShape->GetPosition(1);
	pcValue = pcPosition->GetX()->DoubleValue();
	AssertDouble(2.0, pcValue, 2);
	pcValue = pcPosition->GetY()->DoubleValue();
	AssertDouble(3.0, pcValue, 2);

	pcPosition = pcShape->GetPosition(2);
	pcValue = pcPosition->GetX()->DoubleValue();
	AssertDouble(4.0, pcValue, 2);
	pcValue = pcPosition->GetY()->DoubleValue();
	AssertDouble(5.0, pcValue, 2);

	pcPosition = pcShape->GetPosition(3);
	pcValue = pcPosition->GetX()->DoubleValue();
	AssertDouble(6.0, pcValue, 2);
	pcValue = pcPosition->GetY()->DoubleValue();
	AssertDouble(7.0, pcValue, 2);

	pcPosition = pcShape->GetPosition(4);
	pcValue = pcPosition->GetX()->DoubleValue();
	AssertDouble(8.0, pcValue, 2);
	pcValue = pcPosition->GetY()->DoubleValue();
	AssertDouble(9.0, pcValue, 2);

	pcPosition = pcShape->GetPosition(5);
	pcValue = pcPosition->GetX()->DoubleValue();
	AssertDouble(10.0, pcValue, 2);
	pcValue = pcPosition->GetY()->DoubleValue();
	AssertDouble(11.0, pcValue, 2);

	pcPosition = pcShape->GetPosition(6);
	pcValue = pcPosition->GetX()->DoubleValue();
	AssertDouble(12.0, pcValue, 2);
	pcValue = pcPosition->GetY()->DoubleValue();
	AssertDouble(13.0, pcValue, 2);

	pcValue = pcShape->GetRotation()->DoubleValue();
	AssertDouble(45.235456, pcValue, 6);

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserCommentMacro(void)
{
	CChars								szGerberFile;
	CGerberParser						cParser;
	CGerberCommands						cCommands;
	TRISTATE							tResult;
	size								uiNumCommands;
	CGerberCommand*						pcCommand;
	CGerberCommandApertureMacro*		pcApetureMacro;
	size								uiNumPrimitives;
	CGerberApertureMacro*				pcPrimitive;
	CGerberApertureMacroComment*		pcComment;

	szGerberFile.Init(
		"%AMComment*\n"
		"0 Rectangle with rounded corners, with rotation*\n"
		"0 The origin of the aperture is its center*\n"
		"0 $1 X-size*\n"
		"0 $2 Y-size*\n"
		"0 $3 Rounding radius*\n"
		"0 $4 Rotation angle, in degrees counterclockwise*%\n"
		"M02*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(2, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsApertureMacro());

	pcApetureMacro = (CGerberCommandApertureMacro*)pcCommand;
	uiNumPrimitives = pcApetureMacro->NumPrimitives();
	AssertSize(6, uiNumPrimitives);

	pcPrimitive = pcApetureMacro->GetPrimitive(0);
	AssertTrue(pcPrimitive->IsComment());
	pcComment = (CGerberApertureMacroComment*)pcPrimitive;
	AssertString("Rectangle with rounded corners, with rotation", pcComment->Text());

	pcPrimitive = pcApetureMacro->GetPrimitive(1);
	AssertTrue(pcPrimitive->IsComment());
	pcComment = (CGerberApertureMacroComment*)pcPrimitive;
	AssertString("The origin of the aperture is its center", pcComment->Text());

	pcPrimitive = pcApetureMacro->GetPrimitive(2);
	AssertTrue(pcPrimitive->IsComment());
	pcComment = (CGerberApertureMacroComment*)pcPrimitive;
	AssertString("$1 X-size", pcComment->Text());

	pcPrimitive = pcApetureMacro->GetPrimitive(3);
	AssertTrue(pcPrimitive->IsComment());
	pcComment = (CGerberApertureMacroComment*)pcPrimitive;
	AssertString("$2 Y-size", pcComment->Text());

	pcPrimitive = pcApetureMacro->GetPrimitive(4);
	AssertTrue(pcPrimitive->IsComment());
	pcComment = (CGerberApertureMacroComment*)pcPrimitive;
	AssertString("$3 Rounding radius", pcComment->Text());

	pcPrimitive = pcApetureMacro->GetPrimitive(5);
	AssertTrue(pcPrimitive->IsComment());
	pcComment = (CGerberApertureMacroComment*)pcPrimitive;
	AssertString("$4 Rotation angle, in degrees counterclockwise", pcComment->Text());

	pcCommand = cCommands.GetCommand(1);
	AssertTrue(pcCommand->IsEndOfFile());

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserPolygonMacro(void)
{
	CChars							szGerberFile;
	CGerberParser					cParser;
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	size							uiNumCommands;
	CGerberCommand*					pcCommand;
	CGerberCommandApertureMacro*	pcApetureMacro;
	size							uiNumPrimitives;
	CGerberApertureMacro*			pcPrimitive;
	CGerberApertureMacroPolygon*	pcPolygon;
	CNumber*						pcNumber;

	szGerberFile.Init(
		"%AM Polygon*\n"
		"5,1,8,0,0,7,0*%\n"
		"M02*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(2, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsApertureMacro());

	pcApetureMacro = (CGerberCommandApertureMacro*)pcCommand;
	uiNumPrimitives = pcApetureMacro->NumPrimitives();
	AssertSize(1, uiNumPrimitives);

	pcPrimitive = pcApetureMacro->GetPrimitive(0);
	AssertTrue(pcPrimitive->IsPolygon());

	pcPolygon = (CGerberApertureMacroPolygon*)pcPrimitive;
	pcNumber = pcPolygon->GetCenterX()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(0.f, pcNumber->FloatValue(), 2);

	pcNumber = pcPolygon->GetCenterY()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(0.f, pcNumber->FloatValue(), 2);

	pcNumber = pcPolygon->GetDiameter()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(7.f, pcNumber->FloatValue(), 2);

	pcNumber = pcPolygon->GetNumberOfVertices()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertInt(8, pcNumber->IntValue());

	pcNumber = pcPolygon->GetRotation()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(0.f, pcNumber->FloatValue(), 5);

	pcNumber = pcPolygon->GetExposure()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(1.f, pcNumber->FloatValue(), 5);

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserThermalMacro(void)
{
	CChars							szGerberFile;
	CGerberParser					cParser;
	CGerberCommands					cCommands;
	TRISTATE						tResult;
	size							uiNumCommands;
	CGerberCommand*					pcCommand;
	CGerberCommandApertureMacro*	pcApetureMacro;
	size							uiNumPrimitives;
	CGerberApertureMacro*			pcPrimitive;
	CGerberApertureMacroThermal*	pcThermal;
	CNumber*						pcNumber;

	szGerberFile.Init(
		"%AM Thermal*\n"
		"7,1,2,0.95,0.75,0.175,3.0*%\n"
		"M02*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	uiNumCommands = cCommands.NumCommands();
	AssertSize(2, uiNumCommands);

	pcCommand = cCommands.GetCommand(0);
	AssertTrue(pcCommand->IsApertureMacro());

	pcApetureMacro = (CGerberCommandApertureMacro*)pcCommand;
	uiNumPrimitives = pcApetureMacro->NumPrimitives();
	AssertSize(1, uiNumPrimitives);

	pcPrimitive = pcApetureMacro->GetPrimitive(0);
	AssertTrue(pcPrimitive->IsThermal());

	pcThermal = (CGerberApertureMacroThermal*)pcPrimitive;
	pcNumber = pcThermal->GetCenterX()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(1.f, pcNumber->FloatValue(), 2);

	pcNumber = pcThermal->GetCenterY()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(2.f, pcNumber->FloatValue(), 2);

	pcNumber = pcThermal->GetInnerDiameter()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(0.75f, pcNumber->FloatValue(), 2);

	pcNumber = pcThermal->GetOuterDiameter()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(0.95f, pcNumber->FloatValue(), 2);

	pcNumber = pcThermal->GetRotation()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(3.f, pcNumber->FloatValue(), 5);

	pcNumber = pcThermal->GetGapThickness()->GetConstNumber();
	AssertNotNull(pcNumber);
	AssertFloat(0.175f, pcNumber->FloatValue(), 5);

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserExample1(void)
{
	CChars				szGerberFile;
	CGerberParser		cParser;
	CGerberCommands		cCommands;
	TRISTATE			tResult;

	szGerberFile.Init(
		"%FSLAX26Y26*%\n"
		"%MOMM*%\n"
		"%ADD100C,1.5*%\n"
		"D100*\n"
		"X0Y0D03*\n"
		"M02*\n");
	
	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	cParser.Kill();

	AssertTritrue(tResult);
	AssertSize(6, cCommands.NumCommands());
	AssertTrue(cCommands.GetCommand(0)->IsFormatSpecifier());
	AssertTrue(cCommands.GetCommand(1)->IsMeasurementMode());
	AssertTrue(cCommands.GetCommand(2)->IsApertureDefine());
	AssertTrue(cCommands.GetCommand(3)->IsSetAperture());
	AssertTrue(cCommands.GetCommand(4)->IsType(GC_D03));
	AssertTrue(cCommands.GetCommand(5)->IsEndOfFile());

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParserPartialKiCadReal(void)
{
	CChars				szGerberFile;
	CGerberParser		cParser;
	CGerberCommands		cCommands;
	TRISTATE			tResult;

	szGerberFile.Init(
		"G04 Gerber Test File*\n"
		"%TF.GenerationSoftware,KiCad,Pcbnew,8.0.7*%\n"
		"%TF.CreationDate,2025-02-09T12:46:03+02:00*%\n"
		"%TF.ProjectId,EPM570 Programmer,45504d35-3730-4205-9072-6f6772616d6d,V0*%\n"
		"%TF.SameCoordinates,Original*%\n"
		"%TF.FileFunction,Copper,L1,Top*%\n"
		"%TF.FilePolarity,Positive*%\n"
		"%FSLAX46Y46*%\n"
		"G04 Gerber Fmt 4.6, Leading zero omitted, Abs format (unit mm)*\n"
		"G04 Created by KiCad (PCBNEW 8.0.7) date 2025-02-09 12:46:03*\n"
		"%MOMM*%\n"
		"%LPD*%\n"
		"G01*\n"
		"G04 APERTURE LIST*\n"
		"G04 Aperture macros list*\n"
		"%AMRoundRect*\n"
		"0 Rectangle with rounded corners*\n"
		"0 $1 Rounding radius*\n"
		"0 $2 $3 $4 $5 $6 $7 $8 $9 X,Y pos of 4 corners*\n"
		"0 Add a 4 corners polygon primitive as box body*\n"
		"4,1,4,$2,$3,$4,$5,$6,$7,$8,$9,$2,$3,0*\n"
		"0 Add four circle primitives for the rounded corners*\n"
		"1,1,$1+$1,$2,$3*\n"
		"1,1,$1+$1,$4,$5*\n"
		"1,1,$1+$1,$6,$7*\n"
		"1,1,$1+$1,$8,$9*\n"
		"0 Add four rect primitives between the rounded corners*\n"
		"20,1,$1+$1,$2,$3,$4,$5,0*\n"
		"20,1,$1+$1,$4,$5,$6,$7,0*\n"
		"20,1,$1+$1,$6,$7,$8,$9,0*\n"
		"20,1,$1+$1,$8,$9,$2,$3,0*%\n"
		"G04 Aperture macros list end*\n"
		"%TA.AperFunction,EtchedComponent*%\n"
		"%ADD10C,0.000000*%\n"
		"%TD*%\n"
		"%TA.AperFunction,SMDPad,CuDef*%\n"
		"%ADD11R,1.150000X0.600000*%\n"
		"%TD*%\n"
		"%TA.AperFunction,SMDPad,CuDef*%\n"
		"%ADD12RoundRect,0.250000X-0.475000X0.250000X-0.475000X-0.250000X0.475000X-0.250000X0.475000X0.250000X0*%\n"
		"%TD*%\n"
		"M02*\n");

	cCommands.Init();
	cParser.Init(szGerberFile.Text(), szGerberFile.Length(), "none.txt", &cCommands);
	tResult = cParser.Parse();
	AssertTritrue(tResult);

	cParser.Kill();

	AssertSize(27, cCommands.NumCommands());
	AssertTrue(cCommands.GetCommand(25)->IsDeleteAttribute());
	AssertTrue(cCommands.GetCommand(26)->IsEndOfFile());

	cCommands.Kill();

	szGerberFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberParser(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	DataIOInit();
	ObjectsInit();
	
	TestGerberParserCommandEndOfFile();			//M02
	TestGerberParserCommandComment();			//G04
	TestGerberParserCommandFormatSpecifier();	//FS
	TestGerberParserCommandMeasurementMode();	//MO
	TestGerberParserCommandPlotMode();			//G01, G02, G03
	TestGerberParserCommandLoadPolarity();		//LP
	TestGerberParserCommandLoadMirroring();		//LM
	TestGerberParserCommandLoadRotation();		//LR
	TestGerberParserCommandLoadScaling();		//LS
	TestGerberParserCommandFileAttribute();		//TF
	TestGerberParserCommandApertureAttribute(); //TA
	TestGerberParserCommandObjectAttribute();	//TO
	TestGerberParserCommandDeleteAttribute();	//TD
	TestGerberParserCommandApertureDefine();	//AD
	TestGerberParserCommandSetAperture();		//Dnn
	TestGerberParserCommandOperation();		//D01, D02, D03
	TestGerberParserCommandQuadrantMode();	//G74, G75
	TestGerberParserRegion();				//G36, G37
	TestGerberParserApertureBlock();		//AB
	TestGerberParserStepAndRepeat();		//SR
	TestGerberParserCarriageReturns();
	TestGerberParserCircleMacro();
	TestGerberParserVectorLineMacro();
	TestGerberParserCenterLineMacro();
	TestGerberParserOutlineMacro();
	TestGerberParserCommentMacro();
	TestGerberParserPolygonMacro();
	TestGerberParserThermalMacro();
	TestGerberParserVariableDefinitionMacro();
	TestGerberParserBadExpressionMacro();
	TestGerberParserExample1();
	TestGerberParserPartialKiCadReal();
	TestGerberParserUcamcoExamples();

	ObjectsKill();
	DataIOKill();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

