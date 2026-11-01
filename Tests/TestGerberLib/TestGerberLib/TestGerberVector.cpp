#include <math.h>
#include "BaseLib/TextFile.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "GerberLib/GerberParser.h"
#include "GerberLib/GerberInterpreter.h"
#include "GerberLib/GerberVectoriser.h"
#include "GerberLib/GerberSvgWriter.h"
#include "GerberLib/ExcellonParser.h"
#include "GerberLib/ExcellonInterpreter.h"
#include "GerberLib/ExcellonVectoriser.h"
#include "TestLib/Assert.h"


TRISTATE ParseGerberText(char* szText, CGerberCommands* pcCommands);
TRISTATE ParseGerberFile(char* szDirectory, char* szName, CGerberCommands* pcCommands);
TRISTATE ParseExcellonFile(char* szName, CExcellonCommands* pcCommands);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void VectoriseGerberCommands(CGerberCommands* pcCommands, CGerberVectorImage* pcImage)
{
	CGerberInterpreter	cInterpreter;
	CGerberVectoriser	cVectoriser;

	cVectoriser.Init(pcImage);
	cInterpreter.Init(pcCommands, &cVectoriser);
	AssertTrue(cInterpreter.Interpret());
	cInterpreter.Kill();
	AssertFalse(cVectoriser.HasError());
	cVectoriser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void VectoriseGerberText(char* szText, CGerberVectorImage* pcImage)
{
	CGerberCommands		cCommands;

	cCommands.Init();
	AssertTritrue(ParseGerberText(szText, &cCommands));
	VectoriseGerberCommands(&cCommands, pcImage);
	cCommands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertBounds(CGerberVectorObject* pcObject, float64 fMinX, float64 fMinY, float64 fMaxX, float64 fMaxY)
{
	AssertTrue(pcObject->HasBounds());
	AssertDouble(fMinX, pcObject->GetMinimum()->x, 6);
	AssertDouble(fMinY, pcObject->GetMinimum()->y, 6);
	AssertDouble(fMaxX, pcObject->GetMaximum()->x, 6);
	AssertDouble(fMaxY, pcObject->GetMaximum()->y, 6);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CountContours(CGerberVectorObject* pcObject)
{
	size	i;
	size	uiContours;

	uiContours = 0;
	for (i = 0; i < pcObject->NumSegments(); i++)
	{
		if (pcObject->GetSegment(i)->eType == VS_Move)
		{
			uiContours++;
		}
	}
	return uiContours;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 DiskCornerArea(float64 fRadius, float64 fHalfGap)
{
	float64	fEnd;

	//The area of the disk with x > fHalfGap and y > fHalfGap; the integral of sqrt(R^2 - x^2) - a from a to sqrt(R^2 - a^2).
	if (fRadius * fRadius <= 2 * fHalfGap * fHalfGap)
	{
		return 0;
	}
	fEnd = sqrt(fRadius * fRadius - fHalfGap * fHalfGap);
	return ((fEnd * sqrt(fRadius * fRadius - fEnd * fEnd) + fRadius * fRadius * asin(fEnd / fRadius)) -
			(fHalfGap * sqrt(fRadius * fRadius - fHalfGap * fHalfGap) + fRadius * fRadius * asin(fHalfGap / fRadius))) / 2 -
			fHalfGap * (fEnd - fHalfGap);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberVectorApertures(void)
{
	CGerberVectorImage	cImage;

	cImage.Init();
	VectoriseGerberText(
		"%FSLAX26Y26*%\n"
		"%MOMM*%\n"
		"%ADD10C,2*%\n"
		"%ADD11C,2X1*%\n"
		"%ADD12R,1X2*%\n"
		"%ADD13R,1X2X0.5*%\n"
		"%ADD14O,3X1*%\n"
		"%ADD15O,1X3*%\n"
		"%ADD16P,2X6*%\n"
		"%ADD17P,2X4X45*%\n"
		"%ADD18O,1X1*%\n"
		"D10*\nX0Y0D03*\n"
		"D11*\nX10000000Y0D03*\n"
		"D12*\nX20000000Y0D03*\n"
		"D13*\nX30000000Y0D03*\n"
		"D14*\nX40000000Y0D03*\n"
		"D15*\nX50000000Y0D03*\n"
		"D16*\nX60000000Y0D03*\n"
		"D17*\nX70000000Y10000000D03*\n"
		"D18*\nX80000000Y0D03*\n", &cImage);

	AssertSize(9, cImage.NumObjects());

	AssertDouble(N_PI_64, cImage.GetObject(0)->Area(), 6);
	AssertBounds(cImage.GetObject(0), -1, -1, 1, 1);

	//The hole is a contour winding the other way.
	AssertDouble(N_PI_64 * 0.75, cImage.GetObject(1)->Area(), 6);
	AssertSize(2, CountContours(cImage.GetObject(1)));

	AssertDouble(2, cImage.GetObject(2)->Area(), 6);
	AssertBounds(cImage.GetObject(2), 19.5, -1, 20.5, 1);
	AssertDouble(2 - N_PI_64 * 0.0625, cImage.GetObject(3)->Area(), 6);

	AssertDouble(2 + N_PI_64 * 0.25, cImage.GetObject(4)->Area(), 6);
	AssertBounds(cImage.GetObject(4), 38.5, -0.5, 41.5, 0.5);
	AssertDouble(2 + N_PI_64 * 0.25, cImage.GetObject(5)->Area(), 6);
	AssertBounds(cImage.GetObject(5), 49.5, -1.5, 50.5, 1.5);

	AssertDouble(3 * sqrt(3.0) / 2, cImage.GetObject(6)->Area(), 6);
	AssertBounds(cImage.GetObject(6), 59, -sqrt(3.0) / 2, 61, sqrt(3.0) / 2);

	//Four vertices rotated 45 degrees is a square on the axes.
	AssertDouble(2, cImage.GetObject(7)->Area(), 6);
	AssertBounds(cImage.GetObject(7), 70 - sqrt(0.5), 10 - sqrt(0.5), 70 + sqrt(0.5), 10 + sqrt(0.5));

	AssertDouble(N_PI_64 * 0.25, cImage.GetObject(8)->Area(), 6);

	cImage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberVectorStrokes(void)
{
	CGerberVectorImage	cImage;

	cImage.Init();
	VectoriseGerberText(
		"%FSLAX26Y26*%\n"
		"%MOMM*%\n"
		"%ADD10C,0.1*%\n"
		"%ADD11C,0.2*%\n"
		"%ADD12C,0*%\n"
		"D10*\n"
		"G01*\n"
		"X0Y0D02*\n"
		"X1000000D01*\n"
		"D01*\n"
		"G75*\n"
		"D11*\n"
		"X1000000Y0D02*\n"
		"G03*\n"
		"X0Y1000000I-1000000J0D01*\n"
		"X0Y1000000I0J-1000000D01*\n"
		"G02*\n"
		"X1000000Y0I0J-1000000D01*\n"
		"D12*\n"
		"G01*\n"
		"X5000000D01*\n", &cImage);

	//The zero width draw is an object without an image; it has no contours and no bounds.
	AssertSize(6, cImage.NumObjects());
	AssertSize(0, CountContours(cImage.GetObject(5)));
	AssertFalse(cImage.GetObject(5)->HasBounds());

	AssertDouble(0.1 + N_PI_64 * 0.0025, cImage.GetObject(0)->Area(), 6);
	AssertBounds(cImage.GetObject(0), -0.05, -0.05, 1.05, 0.05);

	//A zero length draw is a circle.
	AssertDouble(N_PI_64 * 0.0025, cImage.GetObject(1)->Area(), 6);
	AssertBounds(cImage.GetObject(1), 0.95, -0.05, 1.05, 0.05);

	//A quarter arc: an annular sector and two half circle caps.
	AssertDouble(N_PI_64 * 0.11, cImage.GetObject(2)->Area(), 6);
	AssertBounds(cImage.GetObject(2), -0.1, -0.1, 1.1, 1.1);

	//A full circle is a ring.
	AssertDouble(N_PI_64 * 0.4, cImage.GetObject(3)->Area(), 6);
	AssertSize(2, CountContours(cImage.GetObject(3)));
	AssertBounds(cImage.GetObject(3), -1.1, -1.1, 1.1, 1.1);

	//A clockwise quarter winds the other way.
	AssertDouble(-N_PI_64 * 0.11, cImage.GetObject(4)->Area(), 6);
	AssertBounds(cImage.GetObject(4), -0.1, -0.1, 1.1, 1.1);

	cImage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberVectorRegions(void)
{
	CGerberVectorImage	cImage;

	//A square with a half circle on top, and a clear square with a square hole made by a cut-in.
	cImage.Init();
	VectoriseGerberText(
		"%FSLAX26Y26*%\n"
		"%MOMM*%\n"
		"G75*\n"
		"G36*\n"
		"X0Y0D02*\n"
		"G01*\n"
		"X2000000D01*\n"
		"Y2000000D01*\n"
		"G03*\n"
		"X0I-1000000J0D01*\n"
		"G01*\n"
		"Y0D01*\n"
		"G37*\n"
		"%LPC*%\n"
		"G36*\n"
		"X10000000Y0D02*\n"
		"X14000000D01*\n"
		"Y4000000D01*\n"
		"X10000000D01*\n"
		"Y0D01*\n"
		"X11000000Y1000000D01*\n"
		"Y3000000D01*\n"
		"X13000000D01*\n"
		"Y1000000D01*\n"
		"X11000000D01*\n"
		"X10000000Y0D01*\n"
		"G37*\n", &cImage);

	AssertSize(2, cImage.NumObjects());
	AssertTrue(cImage.GetObject(0)->IsDark());
	AssertDouble(4 + N_PI_64 / 2, cImage.GetObject(0)->Area(), 6);
	AssertBounds(cImage.GetObject(0), 0, 0, 2, 3);

	AssertFalse(cImage.GetObject(1)->IsDark());
	AssertTrue(cImage.GetObject(1)->GetPolarity() == GP_Clear);
	AssertDouble(16 - 4, cImage.GetObject(1)->Area(), 6);
	AssertSize(1, CountContours(cImage.GetObject(1)));

	cImage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberVectorTransforms(void)
{
	CGerberVectorImage	cImage;

	cImage.Init();
	VectoriseGerberText(
		"%FSLAX26Y26*%\n"
		"%MOMM*%\n"
		"%ADD10R,1X2*%\n"
		"%ADD11P,2X3*%\n"
		"%ADD12C,2*%\n"
		"%ADD13C,0.1*%\n"
		"D10*\n"
		"%LR90*%\n"
		"X5000000Y5000000D03*\n"
		"%LR0*%\n"
		"D11*\n"
		"X0Y0D03*\n"
		"%LMX*%\n"
		"X0Y0D03*\n"
		"%LMN*%\n"
		"%LR30*%\n"
		"%LS2*%\n"
		"D12*\n"
		"X0Y0D03*\n"
		"D13*\n"
		"G01*\n"
		"X0Y0D02*\n"
		"X1000000D01*\n", &cImage);

	AssertSize(5, cImage.NumObjects());

	//Rotated 90 degrees the 1 x 2 rectangle is 2 x 1.
	AssertBounds(cImage.GetObject(0), 4, 4.5, 6, 5.5);
	AssertDouble(2, cImage.GetObject(0)->Area(), 6);

	//The triangle has a vertex on +X; mirrored in X it is on -X.
	AssertBounds(cImage.GetObject(1), -0.5, -sqrt(3.0) / 2, 1, sqrt(3.0) / 2);
	AssertBounds(cImage.GetObject(2), -1, -sqrt(3.0) / 2, 0.5, sqrt(3.0) / 2);
	AssertDouble(fabs(cImage.GetObject(1)->Area()), fabs(cImage.GetObject(2)->Area()), 6);

	//Scaling applies to flashes and to the stroke width.
	AssertDouble(N_PI_64 * 4, cImage.GetObject(3)->Area(), 6);
	AssertBounds(cImage.GetObject(4), -0.1, -0.1, 1.1, 0.1);

	cImage.Kill();

	//Inches are converted to millimetres.
	cImage.Init();
	VectoriseGerberText(
		"%FSLAX26Y26*%\n"
		"%MOIN*%\n"
		"%ADD10C,0.1*%\n"
		"D10*\n"
		"X1000000Y-2000000D03*\n", &cImage);
	AssertDouble(N_PI_64 * 1.27 * 1.27, cImage.GetObject(0)->Area(), 6);
	AssertBounds(cImage.GetObject(0), 25.4 - 1.27, -50.8 - 1.27, 25.4 + 1.27, -50.8 + 1.27);
	cImage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberVectorMacros(void)
{
	CGerberVectorImage		cImage;
	CGerberVectorObject*	pcObject;
	float64					fQuarter;

	cImage.Init();
	VectoriseGerberText(
		"%FSLAX26Y26*%\n"
		"%MOMM*%\n"
		"%AMTHERMAL*7,0,0,0.800,0.550,0.125,45*%\n"
		"%AMVECTOR*20,1,0.5,0,0,2,0,90*%\n"
		"%AMCENTER*21,1,2,1,1,0,90*%\n"
		"%AMPOLYGON*5,1,4,0,0,2,0*%\n"
		"%AMOUTLINE*4,1,3,0,0,3,0,0,2,0,0,0*%\n"
		"%AMDONUT*1,1,$1,0,0*1,0,$2,0,0*%\n"
		"%AMCIRCLES*1,1,1,2,0,90*%\n"
		"%ADD10THERMAL*%\n"
		"%ADD11VECTOR*%\n"
		"%ADD12CENTER*%\n"
		"%ADD13POLYGON*%\n"
		"%ADD14OUTLINE*%\n"
		"%ADD15DONUT,2X1*%\n"
		"%ADD16CIRCLES*%\n"
		"D10*\nX0Y0D03*\n"
		"D11*\nX0Y0D03*\n"
		"D12*\nX0Y0D03*\n"
		"D13*\nX0Y0D03*\n"
		"D14*\nX0Y0D03*\n"
		"D15*\nX0Y0D03*\n"
		"D16*\nX0Y0D03*\n", &cImage);

	AssertSize(7, cImage.NumObjects());

	//Four quarter pieces; the area of the ring outside the gaps.
	pcObject = cImage.GetObject(0);
	AssertSize(4, CountContours(pcObject));
	fQuarter = DiskCornerArea(0.4, 0.0625) - DiskCornerArea(0.275, 0.0625);
	AssertDouble(4 * fQuarter, pcObject->Area(), 6);
	AssertFalse(pcObject->HasExposureOff());

	//The vector line ends are square and the primitive rotates about the macro origin.
	pcObject = cImage.GetObject(1);
	AssertDouble(1, pcObject->Area(), 6);
	AssertBounds(pcObject, -0.25, 0, 0.25, 2);

	pcObject = cImage.GetObject(2);
	AssertDouble(2, pcObject->Area(), 6);
	AssertBounds(pcObject, -0.5, 0, 0.5, 2);

	pcObject = cImage.GetObject(3);
	AssertDouble(2, pcObject->Area(), 6);
	AssertBounds(pcObject, -1, -1, 1, 1);

	pcObject = cImage.GetObject(4);
	AssertDouble(3, fabs(pcObject->Area()), 6);
	AssertBounds(pcObject, 0, 0, 3, 2);

	//Exposure off erases inside the object.
	pcObject = cImage.GetObject(5);
	AssertTrue(pcObject->HasExposureOff());
	AssertSize(2, CountContours(pcObject));

	pcObject = cImage.GetObject(6);
	AssertBounds(pcObject, -0.5, 1.5, 0.5, 2.5);

	cImage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberVectorBlocks(void)
{
	CGerberVectorImage	cImage;

	cImage.Init();
	VectoriseGerberText(
		"%FSLAX26Y26*%\n"
		"%MOMM*%\n"
		"%ADD10C,2*%\n"
		"%ADD11C,1*%\n"
		"%ABD20*%\n"
		"D10*\n"
		"X1000000Y0D03*\n"
		"%LPC*%\n"
		"D11*\n"
		"X1000000Y0D03*\n"
		"%AB*%\n"
		"%LPD*%\n"
		"D20*\n"
		"X10000000Y10000000D03*\n"
		"%LPC*%\n"
		"%LR90*%\n"
		"X20000000Y10000000D03*\n"
		"%LR0*%\n"
		"%LPD*%\n"
		"%SRX3Y2I5.0J4.0*%\n"
		"D11*\n"
		"X0Y0D03*\n"
		"%SR*%\n", &cImage);

	//The block is defined, not drawn; each flash places its two objects.
	AssertSize(2 + 2 + 6, cImage.NumObjects());
	AssertTrue(cImage.GetObject(0)->IsDark());
	AssertFalse(cImage.GetObject(1)->IsDark());
	AssertBounds(cImage.GetObject(0), 10, 9, 12, 11);
	AssertBounds(cImage.GetObject(1), 10.5, 9.5, 11.5, 10.5);

	//A clear flash toggles the block's polarities; rotated about the block origin.
	AssertFalse(cImage.GetObject(2)->IsDark());
	AssertTrue(cImage.GetObject(3)->IsDark());
	AssertBounds(cImage.GetObject(2), 19, 10, 21, 12);

	//Step and repeat steps the whole block first in Y and then in X.
	AssertBounds(cImage.GetObject(4), -0.5, -0.5, 0.5, 0.5);
	AssertBounds(cImage.GetObject(5), -0.5, 3.5, 0.5, 4.5);
	AssertBounds(cImage.GetObject(6), 4.5, -0.5, 5.5, 0.5);
	AssertBounds(cImage.GetObject(7), 4.5, 3.5, 5.5, 4.5);
	AssertBounds(cImage.GetObject(9), 9.5, 3.5, 10.5, 4.5);

	cImage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertVectorisedFile(char* szDirectory, char* szName, size uiObjects)
{
	CGerberCommands		cCommands;
	CGerberVectorImage	cImage;
	CGerberSvgWriter	cWriter;
	CChars				sz;

	cCommands.Init();
	AssertTritrue(ParseGerberFile(szDirectory, szName, &cCommands));
	cImage.Init();
	VectoriseGerberCommands(&cCommands, &cImage);
	AssertSize(uiObjects, cImage.NumObjects());

	sz.Init();
	cWriter.Init();
	cWriter.Write(&sz, &cImage, "black", "white");
	cWriter.Kill();
	AssertTrue(sz.StartsWith("<svg "));
	AssertTrue(sz.EndsWith("</svg>\n"));
	sz.Kill();

	cImage.Kill();
	cCommands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberVectorFiles(void)
{
	CExcellonCommands		cDrills;
	CExcellonInterpreter	cInterpreter;
	CExcellonVectoriser		cVectoriser;
	CGerberVectorImage		cImage;

	AssertVectorisedFile("Ucamco", "2-13-1_Two_square_boxes.gbr", 8);
	AssertVectorisedFile("Ucamco", "2-13-2_Polarities_and_Apertures.gbr", 23);
	AssertVectorisedFile("Ucamco", "4-11-6_Block_with_different_orientations.gbr", 20);
	AssertVectorisedFile("Ucamco", "4-6-4_Nested_blocks.gbr", 2 + 6 * (6 * 4 * 3 + 1));
	AssertVectorisedFile("Ucamco", "6-1-6-2_A_drill_file.gbr", 29);
	AssertVectorisedFile("Ucamco", "SMD_prim_20.gbr", 8);
	AssertVectorisedFile("Ucamco", "SMD_prim_21_X1.gbr", 8);
	AssertVectorisedFile("Ucamco", "sample_macro.gbr", 401);

	//The object counts agree with gerbonara and with counting the D01s outside regions, the D03s, the regions, the blocks and the repeats.
	AssertVectorisedFile("KiCad" _FS_ "Adder 16bit", "Adder 16bit-F_Cu.gtl", 959);
	AssertVectorisedFile("KiCad" _FS_ "Adder 16bit", "Adder 16bit-B_Cu.gbl", 690);
	AssertVectorisedFile("KiCad" _FS_ "Adder 16bit", "Adder 16bit-F_Mask.gts", 132);
	AssertVectorisedFile("KiCad" _FS_ "Adder 16bit", "Adder 16bit-B_Mask.gbs", 56);
	AssertVectorisedFile("KiCad" _FS_ "Adder 16bit", "Adder 16bit-F_Paste.gtp", 76);
	AssertVectorisedFile("KiCad" _FS_ "Adder 16bit", "Adder 16bit-F_Silkscreen.gto", 2242);
	AssertVectorisedFile("KiCad" _FS_ "Adder 16bit", "Adder 16bit-Edge_Cuts.gm1", 10);

	cDrills.Init();
	AssertTritrue(ParseExcellonFile("Adder 16bit" _FS_ "Adder 16bit-PTH.drl", &cDrills));
	cImage.Init();
	cVectoriser.Init(&cImage, true);
	cInterpreter.Init(&cDrills, &cVectoriser);
	AssertTrue(cInterpreter.Interpret());
	cInterpreter.Kill();
	cVectoriser.Kill();
	AssertSize(139, cImage.NumObjects());
	AssertDouble(N_PI_64 * 0.19939 * 0.19939, cImage.GetObject(0)->Area(), 6);
	AssertBounds(cImage.GetObject(0), 1.27 - 0.19939, -64.77 - 0.19939, 1.27 + 0.19939, -64.77 + 0.19939);
	cImage.Kill();
	cDrills.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberSvgWriter(void)
{
	CGerberVectorImage	cImage;
	CGerberSvgWriter	cWriter;
	CChars				sz;

	cImage.Init();
	VectoriseGerberText(
		"%FSLAX26Y26*%\n"
		"%MOMM*%\n"
		"%ADD10R,4X2*%\n"
		"%ADD11C,1*%\n"
		"%AMDONUT*1,1,1,0,0*1,0,0.5,0,0*%\n"
		"%ADD12DONUT*%\n"
		"D10*\n"
		"X0Y0D03*\n"
		"%LPC*%\n"
		"D11*\n"
		"X1000000Y0D03*\n"
		"%LPD*%\n"
		"D12*\n"
		"X3000000Y1000000D03*\n", &cImage);

	sz.Init();
	cWriter.Init();
	cWriter.Write(&sz, &cImage, "#c8a24a", "#0b3d1e");
	cWriter.Kill();

	//Y is negated.  The clear circle masks the rectangle before it; the donut hole masks the donut only.
	AssertString(
		"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"7.5mm\" height=\"4.5mm\" viewBox=\"-3 -2.5 7.5 4.5\">\n"
		"<rect x=\"-3\" y=\"-2.5\" width=\"7.5\" height=\"4.5\" fill=\"#0b3d1e\"/>\n"
		"<g fill=\"#c8a24a\">\n"
		"<defs>\n"
		"<mask id=\"m0\" maskUnits=\"userSpaceOnUse\" x=\"-3\" y=\"-2.5\" width=\"7.5\" height=\"4.5\">\n"
		"<rect x=\"-3\" y=\"-2.5\" width=\"7.5\" height=\"4.5\" fill=\"white\"/>\n"
		"<g fill=\"black\">\n"
		"<path d=\"M1.5 0A0.5 0.5 0 0 0 0.5 0A0.5 0.5 0 0 0 1.5 0Z\"/>\n"
		"</g></mask>\n"
		"</defs>\n"
		"<g mask=\"url(#m0)\">\n"
		"<path d=\"M-2 1L2 1L2 -1L-2 -1L-2 1Z\"/>\n"
		"</g>\n"
		"<defs>\n"
		"<mask id=\"m1\" maskUnits=\"userSpaceOnUse\" x=\"2.499\" y=\"-1.501\" width=\"1.002\" height=\"1.002\">\n"
		"<rect x=\"2.499\" y=\"-1.501\" width=\"1.002\" height=\"1.002\" fill=\"white\"/>\n"
		"<g fill=\"black\">\n"
		"<path d=\"M3.25 -1A0.25 0.25 0 0 0 2.75 -1A0.25 0.25 0 0 0 3.25 -1Z\"/>\n"
		"</g></mask>\n"
		"</defs>\n"
		"<g mask=\"url(#m1)\">\n"
		"<path d=\"M3.5 -1A0.5 0.5 0 0 0 2.5 -1A0.5 0.5 0 0 0 3.5 -1Z\"/>\n"
		"</g>\n"
		"</g>\n"
		"</svg>\n", sz.Text());

	sz.Kill();
	cImage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberVector(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	TestGerberVectorApertures();
	TestGerberVectorStrokes();
	TestGerberVectorRegions();
	TestGerberVectorTransforms();
	TestGerberVectorMacros();
	TestGerberVectorBlocks();
	TestGerberVectorFiles();
	TestGerberSvgWriter();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

