#include "BaseLib/LogToMemory.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "GerberLib/GerberParser.h"
#include "GerberLib/GerberCommands.h"
#include "GerberLib/GerberInterpreter.h"
#include "TestLib/Assert.h"


TRISTATE ParseGerberText(char* szText, CGerberCommands* pcCommands);
TRISTATE ParseGerberFile(char* szName, CGerberCommands* pcCommands);
TRISTATE ParseGerberFile(char* szDirectory, char* szName, CGerberCommands* pcCommands);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
class CTestGerberListener : public CGerberInterpreterListener
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

	void AppendAperture(CGerberGraphicsState* pcState)
	{
		//Contour segments are not stroked so they have no aperture.
		if (!pcState->mbInRegion)
		{
			msz.Append(" D");
			msz.Append(pcState->mpcCurrentAperture->GetDCode());
		}
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
		AppendAperture(pcState);
		AppendPoint(psStart);
		AppendPoint(psEnd);
		msz.AppendNewLine();
	}

	void Arc(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd, SGerberPoint* psCenter, bool bClockwise)
	{
		msz.Append(bClockwise ? "ArcCW" : "ArcCCW");
		AppendAperture(pcState);
		AppendPoint(psStart);
		AppendPoint(psEnd);
		AppendPoint(psCenter);
		msz.AppendNewLine();
	}

	void Flash(CGerberGraphicsState* pcState, SGerberPoint* psPoint)
	{
		msz.Append("Flash");
		AppendAperture(pcState);
		AppendPoint(psPoint);
		if (pcState->mePolarity == GP_Clear)
		{
			msz.Append(" Clear");
		}
		msz.AppendNewLine();
	}

	void BeginRegion(CGerberGraphicsState* pcState)
	{
		msz.Append("BeginRegion");
		msz.AppendNewLine();
	}

	void EndRegion(CGerberGraphicsState* pcState)
	{
		msz.Append("EndRegion");
		msz.AppendNewLine();
	}

	void BeginBlock(CGerberGraphicsState* pcState, CGerberCommandApertureBlock* pcBlock)
	{
		msz.Append("BeginBlock D");
		msz.Append(pcBlock->GetDCode());
		msz.AppendNewLine();
	}

	void EndBlock(CGerberGraphicsState* pcState, CGerberAperture* pcAperture)
	{
		msz.Append("EndBlock D");
		msz.Append(pcAperture->GetDCode());
		msz.AppendNewLine();
	}

	void BeginStepAndRepeat(CGerberGraphicsState* pcState, CGerberCommandStepAndRepeat* pcStepAndRepeat)
	{
		msz.Append("BeginStepAndRepeat ");
		msz.Append(pcStepAndRepeat->GetXRepeats());
		msz.Append("x");
		msz.Append(pcStepAndRepeat->GetYRepeats());
		msz.AppendNewLine();
	}

	void EndStepAndRepeat(CGerberGraphicsState* pcState)
	{
		msz.Append("EndStepAndRepeat");
		msz.AppendNewLine();
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
class CCountGerberListener : public CGerberInterpreterListener
{
public:
	size	muiMoves;
	size	muiDraws;
	size	muiArcs;
	size	muiFlashes;
	size	muiRegions;

	void Init(void)
	{
		muiMoves = 0;
		muiDraws = 0;
		muiArcs = 0;
		muiFlashes = 0;
		muiRegions = 0;
	}

	void Move(CGerberGraphicsState* pcState, SGerberPoint* psPoint)
	{
		muiMoves++;
	}

	void Draw(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd)
	{
		muiDraws++;
	}

	void Arc(CGerberGraphicsState* pcState, SGerberPoint* psStart, SGerberPoint* psEnd, SGerberPoint* psCenter, bool bClockwise)
	{
		muiArcs++;
	}

	void Flash(CGerberGraphicsState* pcState, SGerberPoint* psPoint)
	{
		muiFlashes++;
	}

	void BeginRegion(CGerberGraphicsState* pcState)
	{
		muiRegions++;
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool InterpretGerberText(char* szText, CTestGerberListener* pcListener)
{
	CGerberCommands		cCommands;
	CGerberInterpreter	cInterpreter;
	TRISTATE			tResult;
	bool				bResult;

	cCommands.Init();
	tResult = ParseGerberText(szText, &cCommands);
	AssertTritrue(tResult);

	cInterpreter.Init(&cCommands, pcListener);
	bResult = cInterpreter.Interpret();
	cInterpreter.Kill();

	cCommands.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertGerberInterpreterError(char* szText, char* szExpectedError)
{
	CTestGerberListener	cListener;
	CLogToMemory		cMemoryLog;
	char				szLogText[1000];
	bool				bResult;

	cListener.Init();
	cMemoryLog.Start(true);
	bResult = InterpretGerberText(szText, &cListener);
	cMemoryLog.Stop(szLogText, 1000);
	cListener.Kill();

	AssertFalse(bResult);
	AssertTrue(strstr(szLogText, szExpectedError) != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertEvaluatedPrimitive(CGerberEvaluatedPrimitive* pcPrimitive, EGerberApertureMacroPrimitive ePrimitive, size uiNumValues, const char** aszValues)
{
	size	i;

	AssertTrue(pcPrimitive->IsPrimitive(ePrimitive));
	AssertSize(uiNumValues, pcPrimitive->NumValues());
	for (i = 0; i < uiNumValues; i++)
	{
		AssertNumber(aszValues[i], pcPrimitive->GetValue(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberInterpreterCurrentPoint(void)
{
	CTestGerberListener	cListener;
	CGerberCommands		cCommands;
	CGerberInterpreter	cInterpreter;
	CGerberGraphicsState*	pcState;

	//Example 2.13.1 from the specification.  Y and X are modal.
	cCommands.Init();
	AssertTritrue(ParseGerberFile("2-13-1_Two_square_boxes.gbr", &cCommands));
	cListener.Init();
	cInterpreter.Init(&cCommands, &cListener);
	AssertTrue(cInterpreter.Interpret());
	AssertString(
		"Move 0,0\n"
		"Draw D10 0,0 5,0\n"
		"Draw D10 5,0 5,5\n"
		"Draw D10 5,5 0,5\n"
		"Draw D10 0,5 0,0\n"
		"Move 6,0\n"
		"Draw D10 6,0 11,0\n"
		"Draw D10 11,0 11,5\n"
		"Draw D10 11,5 6,5\n"
		"Draw D10 6,5 6,0\n", cListener.msz.Text());

	pcState = cInterpreter.GetGraphicsState();
	AssertTrue(pcState->mbCurrentPoint);
	AssertNumber("6", &pcState->msCurrentPoint.cX);
	AssertNumber("0", &pcState->msCurrentPoint.cY);
	AssertTrue(pcState->meUnit == GMM_Millimeters);
	AssertTrue(pcState->mePlotMode == GPM_Linear);
	AssertInt(10, pcState->mpcCurrentAperture->GetDCode());

	cInterpreter.Kill();
	cListener.Kill();
	cCommands.Kill();

	//Arcs, flashes, regions and polarity.
	cListener.Init();
	AssertTrue(InterpretGerberText(
		"%FSLAX26Y26*%\n"
		"%MOMM*%\n"
		"%ADD10C,0.1*%\n"
		"%ADD11R,1X2*%\n"
		"D10*\n"
		"X1000000Y0D02*\n"
		"G75*\n"
		"G03*\n"
		"X0Y1000000I-1000000J0D01*\n"
		"G02*\n"
		"X-1000000Y0I0J-1000000D01*\n"
		"D11*\n"
		"X3000000D03*\n"
		"%LPC*%\n"
		"Y-2000000D03*\n"
		"%LPD*%\n"
		"G01*\n"
		"G36*\n"
		"X0Y0D02*\n"
		"X1000000D01*\n"
		"Y1000000D01*\n"
		"X0Y0D01*\n"
		"G37*\n"
		"M02*\n", &cListener));
	AssertString(
		"Move 1,0\n"
		"ArcCCW D10 1,0 0,1 0,0\n"
		"ArcCW D10 0,1 -1,0 0,0\n"
		"Flash D11 3,0\n"
		"Flash D11 3,-2 Clear\n"
		"BeginRegion\n"
		"Move 0,0\n"
		"Draw 0,0 1,0\n"
		"Draw 1,0 1,1\n"
		"Draw 1,1 0,0\n"
		"EndRegion\n", cListener.msz.Text());
	cListener.Kill();

	//An undefined plot state is linear, as in Ucamco's example files.
	cListener.Init();
	AssertTrue(InterpretGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,0.1*%\nD10*\nX0Y0D02*\nX1000000Y2000000D01*\n", &cListener));
	AssertString("Move 0,0\nDraw D10 0,0 1,2\n", cListener.msz.Text());
	cListener.Kill();

	//The current point is undefined after AB and SR statements.
	cListener.Init();
	AssertTrue(InterpretGerberText(
		"%FSLAX26Y26*%\n"
		"%MOMM*%\n"
		"%ADD10C,0.1*%\n"
		"%ABD20*%\n"
		"D10*\n"
		"X1000000Y1000000D03*\n"
		"%AB*%\n"
		"D20*\n"
		"X5000000Y5000000D03*\n"
		"%SRX3Y2I1.5J2.5*%\n"
		"D10*\n"
		"X0Y0D03*\n"
		"%SR*%\n"
		"X2000000Y2000000D03*\n", &cListener));
	AssertString(
		"BeginBlock D20\n"
		"Flash D10 1,1\n"
		"EndBlock D20\n"
		"Flash D20 5,5\n"
		"BeginStepAndRepeat 3x2\n"
		"Flash D10 0,0\n"
		"EndStepAndRepeat\n"
		"Flash D10 2,2\n", cListener.msz.Text());
	cListener.Kill();

	AssertGerberInterpreterError("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,0.1*%\n%ABD20*%\nD10*\nX1000000Y1000000D03*\n%AB*%\nD10*\nY0D03*\n", "X coordinate omitted but the current point is undefined.");
	AssertGerberInterpreterError("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,0.1*%\nD10*\n%SRX2Y2I1J1*%\nX0Y0D03*\n%SR*%\nX0D03*\n", "Y coordinate omitted but the current point is undefined.");
	AssertGerberInterpreterError("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,0.1*%\nD10*\nG01*\nX0Y0D01*\n", "Plot (D01) needs a current point to start from.");
	AssertGerberInterpreterError("%FSLAX26Y26*%\n%MOMM*%\nX0Y0D02*\nG01*\nX1Y1D01*\n", "Plot (D01) needs a current aperture.");
	AssertGerberInterpreterError("%FSLAX26Y26*%\n%MOMM*%\n%ADD10R,1X1*%\nD10*\nG01*\nX0Y0D02*\nX1Y1D01*\n", "Plot (D01) needs a solid circle aperture but D10 is not.");
	AssertGerberInterpreterError("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,1X0.5*%\nD10*\nG01*\nX0Y0D02*\nX1Y1D01*\n", "Plot (D01) needs a solid circle aperture but D10 is not.");
	AssertGerberInterpreterError("%FSLAX26Y26*%\n%MOMM*%\nX0Y0D03*\n", "Flash (D03) needs a current aperture.");
	AssertGerberInterpreterError("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,0.1*%\nD10*\nX0Y0D02*\nG02*\nX1Y1I1J0D01*\n", "G75 must be issued before the first circular plot (D01).");
	AssertGerberInterpreterError("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,0.1*%\nD10*\nX0Y0D02*\nG74*\nG02*\nX1Y1I1J0D01*\n", "Single quadrant (G74) arcs are not supported.");
	AssertGerberInterpreterError("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,0.1*%\nD10*\nX0Y0D02*\nG75*\nG02*\nX1Y1I1D01*\n", "Circular plot (D01) needs both I and J.");
	AssertGerberInterpreterError("%FSLAX26Y26*%\nX0Y0D02*\n", "Unit (MO) must be set before the first operation.");
	AssertGerberInterpreterError("%MOMM*%\nD02*\n", "Format Specification (FS) must be set before the first operation.");
	AssertGerberInterpreterError("%MOMM*%\n%MOIN*%\n", "Unit (MO) can only be set once.");
	AssertGerberInterpreterError("%FSLAX26Y26*%\n%FSLAX26Y26*%\n", "Format Specification (FS) can only be set once.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberInterpreterApertures(void)
{
	CTestGerberListener	cListener;
	CGerberCommands		cCommands;
	CGerberInterpreter	cInterpreter;
	CGerberApertures*	pcApertures;
	CGerberAperture*	pcAperture;

	cCommands.Init();
	AssertTritrue(ParseGerberText(
		"%ADD10C,0.5*%\n"
		"%ADD11C,1X0.4*%\n"
		"%ADD12R,1X2*%\n"
		"%ADD13O,1X2X0.5*%\n"
		"%ADD14P,1X6*%\n"
		"%ADD15P,1X3X-45X0.2*%\n", &cCommands));
	cListener.Init();
	cInterpreter.Init(&cCommands, &cListener);
	AssertTrue(cInterpreter.Interpret());

	pcApertures = cInterpreter.GetApertures();
	AssertSize(6, pcApertures->NumApertures());

	pcAperture = pcApertures->Get(10);
	AssertTrue(pcAperture->IsType(GAT_Circle));
	AssertTrue(pcAperture->IsSolidCircle());
	AssertNumber("0.5", pcAperture->GetParameter(0));

	pcAperture = pcApertures->Get(11);
	AssertTrue(pcAperture->IsType(GAT_Circle));
	AssertFalse(pcAperture->IsSolidCircle());

	AssertTrue(pcApertures->Get(12)->IsType(GAT_Rectangle));
	AssertTrue(pcApertures->Get(13)->IsType(GAT_Obround));
	AssertTrue(pcApertures->Get(14)->IsType(GAT_Polygon));
	pcAperture = pcApertures->Get(15);
	AssertTrue(pcAperture->IsType(GAT_Polygon));
	AssertSize(4, pcAperture->NumParameters());
	AssertNumber("-45", pcAperture->GetParameter(2));
	AssertNull(pcApertures->Get(16));

	cInterpreter.Kill();
	cListener.Kill();
	cCommands.Kill();

	AssertGerberInterpreterError("%ADD10C,0.5*%\n%ADD10C,0.6*%\n", "Aperture D10 is already defined.");
	AssertGerberInterpreterError("%ADD10C,0.5*%\n%ABD10*%\n%AB*%\n", "Aperture D10 is already defined.");
	AssertGerberInterpreterError("D10*\n", "Aperture D10 is not defined.");
	AssertGerberInterpreterError("%ADD10C*%\n", "Aperture D10 has the wrong number of parameters.");
	AssertGerberInterpreterError("%ADD10C,1X2X3*%\n", "Aperture D10 has the wrong number of parameters.");
	AssertGerberInterpreterError("%ADD10R,1*%\n", "Aperture D10 has the wrong number of parameters.");
	AssertGerberInterpreterError("%ADD10O,1X2X3X4*%\n", "Aperture D10 has the wrong number of parameters.");
	AssertGerberInterpreterError("%ADD10P,1X13*%\n", "Aperture D10 polygon must have 3 to 12 vertices.");
	AssertGerberInterpreterError("%ADD10P,1X3.5*%\n", "Aperture D10 polygon must have 3 to 12 vertices.");
	AssertGerberInterpreterError("%ADD10C,-1*%\n", "Aperture D10 has a negative size.");
	AssertGerberInterpreterError("%ADD10NOTDEFINED,1*%\n", "Aperture macro [NOTDEFINED] is not defined.");
	AssertGerberInterpreterError("%AMBOX*21,1,1,1,0,0,0*%\n%AMBOX*21,1,2,2,0,0,0*%\n", "Aperture macro [BOX] is already defined.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberInterpreterMacros(void)
{
	CTestGerberListener			cListener;
	CGerberCommands				cCommands;
	CGerberInterpreter			cInterpreter;
	CGerberEvaluatedPrimitives*	pcPrimitives;
	CGerberAperture*			pcAperture;

	//Examples 4.5.4.3 and 4.5.5.2 from the specification, BOXR from sample_macro.gbr and RoundRect from KiCad.
	cCommands.Init();
	AssertTritrue(ParseGerberText(
		"%AMDONUTCAL*\n"
		"1,1,$1,$2,$3*\n"
		"$5=$4x0.25*\n"
		"1,0,$5,$2,$3*%\n"
		"%ADD35DONUTCAL,0.020X0X0X0.06*%\n"
		"%AMDONUTVAR*1,1,$1,$2,$3*1,0,$4,$2,$3*%\n"
		"%ADD34DONUTVAR,0.100X0X0X0.080*%\n"
		"%ADD36DONUTVAR,0.100X1.5*%\n"
		"%AMBOXR*\n"
		"0 Rectangle with Rounded corners*\n"
		"$5=$1/2-$3*\n"
		"$6=-$3+$2/2*\n"
		"$7=2x$3*\n"
		"21,1,$1,$2-$7,0,0,$4*\n"
		"21,1,$1-$7,$2,0,0,$4*\n"
		"1,1,$7,$5,$6,$4*\n"
		"1,1,$7,-$5,-$6,$4*%\n"
		"%ADD10BOXR,0.1000X0.0500X0.0200X30.00*%\n"
		"%AMRoundRect*\n"
		"4,1,4,$2,$3,$4,$5,$6,$7,$8,$9,$2,$3,0*\n"
		"1,1,$1+$1,$2,$3*\n"
		"20,1,$1+$1,$2,$3,$4,$5,0*%\n"
		"%ADD12RoundRect,0.250000X-0.475000X0.250000X-0.475000X-0.250000X0.475000X-0.250000X0.475000X0.250000X0*%\n"
		"%AMSHAPES*\n"
		"5,1,8,0,0,(1+1)x2,22.5*\n"
		"7,0,0,0.8,0.55,0.125,45*%\n"
		"%ADD13SHAPES*%\n", &cCommands));
	cListener.Init();
	cInterpreter.Init(&cCommands, &cListener);
	AssertTrue(cInterpreter.Interpret());

	//$4 = 0.06, $5 = 0.06 x 0.25 = 0.015.
	pcAperture = cInterpreter.GetApertures()->Get(35);
	AssertTrue(pcAperture->IsType(GAT_Macro));
	AssertString("DONUTCAL", pcAperture->GetMacro()->NameText());
	pcPrimitives = pcAperture->GetPrimitives();
	AssertSize(2, pcPrimitives->NumPrimitives());
	const char* aszDonutCal0[] = { "1", "0.02", "0", "0", "0" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(0), GAMP_Circle, 5, aszDonutCal0);
	const char* aszDonutCal1[] = { "0", "0.015", "0", "0", "0" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(1), GAMP_Circle, 5, aszDonutCal1);

	pcPrimitives = cInterpreter.GetApertures()->Get(34)->GetPrimitives();
	const char* aszDonutVar1[] = { "0", "0.08", "0", "0", "0" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(1), GAMP_Circle, 5, aszDonutVar1);

	//Undefined variables are 0.
	pcPrimitives = cInterpreter.GetApertures()->Get(36)->GetPrimitives();
	const char* aszDonutVarDefault0[] = { "1", "0.1", "1.5", "0", "0" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(0), GAMP_Circle, 5, aszDonutVarDefault0);
	const char* aszDonutVarDefault1[] = { "0", "0", "1.5", "0", "0" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(1), GAMP_Circle, 5, aszDonutVarDefault1);

	//$5 = 0.05 - 0.02 = 0.03, $6 = -0.02 + 0.025 = 0.005, $7 = 0.04.  The comment adds no primitive.
	pcPrimitives = cInterpreter.GetApertures()->Get(10)->GetPrimitives();
	AssertSize(4, pcPrimitives->NumPrimitives());
	const char* aszBoxR0[] = { "1", "0.1", "0.01", "0", "0", "30" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(0), GAMP_Center_Line, 6, aszBoxR0);
	const char* aszBoxR1[] = { "1", "0.06", "0.05", "0", "0", "30" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(1), GAMP_Center_Line, 6, aszBoxR1);
	const char* aszBoxR2[] = { "1", "0.04", "0.03", "0.005", "30" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(2), GAMP_Circle, 5, aszBoxR2);
	const char* aszBoxR3[] = { "1", "0.04", "-0.03", "-0.005", "30" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(3), GAMP_Circle, 5, aszBoxR3);

	pcPrimitives = cInterpreter.GetApertures()->Get(12)->GetPrimitives();
	AssertSize(3, pcPrimitives->NumPrimitives());
	const char* aszRoundRect0[] = { "1", "4", "-0.475", "0.25", "-0.475", "-0.25", "0.475", "-0.25", "0.475", "0.25", "-0.475", "0.25", "0" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(0), GAMP_Outline, 13, aszRoundRect0);
	const char* aszRoundRect1[] = { "1", "0.5", "-0.475", "0.25", "0" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(1), GAMP_Circle, 5, aszRoundRect1);
	const char* aszRoundRect2[] = { "1", "0.5", "-0.475", "0.25", "-0.475", "-0.25", "0" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(2), GAMP_Vector_Line, 7, aszRoundRect2);

	pcPrimitives = cInterpreter.GetApertures()->Get(13)->GetPrimitives();
	const char* aszPolygon[] = { "1", "8", "0", "0", "4", "22.5" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(0), GAMP_Polygon, 6, aszPolygon);
	const char* aszThermal[] = { "0", "0", "0.8", "0.55", "0.125", "45" };
	AssertEvaluatedPrimitive(pcPrimitives->GetPrimitive(1), GAMP_Thermal_Center, 6, aszThermal);

	cInterpreter.Kill();
	cListener.Kill();
	cCommands.Kill();

	AssertGerberInterpreterError("%AMBAD*$1=2*1,1,$1,0,0*%\n%ADD10BAD,1*%\n", "Aperture macro variable $1 cannot be redefined.");
	AssertGerberInterpreterError("%AMBAD*$2=1*$2=2*%\n%ADD10BAD*%\n", "Aperture macro variable $2 cannot be redefined.");
	AssertGerberInterpreterError("%AMBAD*1,$1,1,0,0*%\n%ADD10BAD,2*%\n", "Aperture macro primitive exposure must be 0 or 1.");
	AssertGerberInterpreterError("%AMBAD*5,1,$1,0,0,1,0*%\n%ADD10BAD,13*%\n", "Aperture macro polygon must have 3 to 12 vertices.");
	AssertGerberInterpreterError("%AMBAD*1,1,1/$1,0,0*%\n%ADD10BAD*%\n", "Could not evaluate aperture macro for aperture D10.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberInterpreterUcamcoExamples(void)
{
	const char*			aszFiles[] = {
		"2-13-1_Two_square_boxes.gbr",
		"2-13-2_Polarities_and_Apertures.gbr",
		"4-11-6_Block_with_different_orientations.gbr",
		"4-6-4_Nested_blocks.gbr",
		"6-1-6-2_A_drill_file.gbr",
		"SMD_prim_20.gbr",
		"SMD_prim_20_X1.gbr",
		"SMD_prim_21.gbr",
		"SMD_prim_21_X1.gbr",
		"sample_macro.gbr",
		"sample_macro_X1.gbr" };
	size				i;
	CTestGerberListener	cListener;
	CGerberCommands		cCommands;
	CGerberInterpreter	cInterpreter;

	for (i = 0; i < 11; i++)
	{
		cCommands.Init();
		AssertTritrue(ParseGerberFile((char*)aszFiles[i], &cCommands));
		cListener.Init();
		cInterpreter.Init(&cCommands, &cListener);
		AssertTrue(cInterpreter.Interpret());
		cInterpreter.Kill();
		cListener.Kill();
		cCommands.Kill();
	}

	//Example 4.11.6: the block is flashed with the current point modal after the flash.
	cCommands.Init();
	AssertTritrue(ParseGerberFile("4-11-6_Block_with_different_orientations.gbr", &cCommands));
	cListener.Init();
	cInterpreter.Init(&cCommands, &cListener);
	AssertTrue(cInterpreter.Interpret());
	AssertString(
		"BeginBlock D12\n"
		"Flash D10 -2.5,-1\n"
		"Flash D10 -2.5,1\n"
		"Flash D11 -2.5,-1 Clear\n"
		"Move -0.5,-1\n"
		"Draw D11 -0.5,-1 2.5,-1\n"
		"ArcCCW D11 2.5,-1 0.5,1 0.5,-1\n"
		"EndBlock D12\n"
		"Flash D12 0,0\n"
		"Flash D12 10,0\n"
		"Flash D12 0,8\n"
		"Flash D12 10,8\n", cListener.msz.Text());
	cInterpreter.Kill();
	cListener.Kill();
	cCommands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertKiCadLayer(char* szName, size uiApertures, size uiMoves, size uiDraws, size uiArcs, size uiFlashes, size uiRegions)
{
	CCountGerberListener	cListener;
	CGerberCommands			cCommands;
	CGerberInterpreter		cInterpreter;

	cCommands.Init();
	AssertTritrue(ParseGerberFile("KiCad" _FS_ "Adder 16bit", szName, &cCommands));
	cListener.Init();
	cInterpreter.Init(&cCommands, &cListener);
	AssertTrue(cInterpreter.Interpret());
	AssertTrue(cInterpreter.GetGraphicsState()->meUnit == GMM_Millimeters);
	AssertSize(uiApertures, cInterpreter.GetApertures()->NumApertures());
	AssertSize(uiMoves, cListener.muiMoves);
	AssertSize(uiDraws, cListener.muiDraws);
	AssertSize(uiArcs, cListener.muiArcs);
	AssertSize(uiFlashes, cListener.muiFlashes);
	AssertSize(uiRegions, cListener.muiRegions);
	cInterpreter.Kill();
	cCommands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberInterpreterKiCadAdder16Bit(void)
{
	//Moves are the D02s, draws plus arcs are the D01s, arcs are the G02s and G03s, flashes are the D03s and regions are the G36s in the files.
	AssertKiCadLayer("Adder 16bit-F_Cu.gtl",			10,  744, 6930, 250, 215, 214);
	AssertKiCadLayer("Adder 16bit-B_Cu.gbl",			7,   551, 3633, 220, 139, 113);
	AssertKiCadLayer("Adder 16bit-F_Mask.gts",			7,   0,   0,    0,   132, 0);
	AssertKiCadLayer("Adder 16bit-B_Mask.gbs",			4,   0,   0,    0,   56,  0);
	AssertKiCadLayer("Adder 16bit-F_Paste.gtp",			3,   0,   0,    0,   76,  0);
	AssertKiCadLayer("Adder 16bit-B_Paste.gbp",			0,   0,   0,    0,   0,   0);
	AssertKiCadLayer("Adder 16bit-F_Silkscreen.gto",	3,   2242, 3246, 0,  0,   8);
	AssertKiCadLayer("Adder 16bit-B_Silkscreen.gbo",	0,   0,   0,    0,   0,   0);
	AssertKiCadLayer("Adder 16bit-Edge_Cuts.gm1",		1,   10,  5,    5,   0,   0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberInterpreter(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	TestGerberInterpreterCurrentPoint();
	TestGerberInterpreterApertures();
	TestGerberInterpreterMacros();
	TestGerberInterpreterUcamcoExamples();
	TestGerberInterpreterKiCadAdder16Bit();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

