#include <math.h>
#include "BaseLib/TypeConverter.h"
#include "BaseLib/NumberControl.h"
#include "GerberLib/GerberParser.h"
#include "GerberLib/GerberInterpreter.h"
#include "GerberLib/GerberVectoriser.h"
#include "GerberLib/GerberRasteriser.h"
#include "GerberLib/ExcellonParser.h"
#include "GerberLib/ExcellonInterpreter.h"
#include "GerberLib/ExcellonVectoriser.h"
#include "TestLib/Assert.h"


void VectoriseGerberText(char* szText, CGerberVectorImage* pcImage);
void VectoriseGerberCommands(CGerberCommands* pcCommands, CGerberVectorImage* pcImage);
TRISTATE ParseGerberFile(char* szDirectory, char* szName, CGerberCommands* pcCommands);
TRISTATE ParseExcellonFile(char* szName, CExcellonCommands* pcCommands);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 RasteriseGerberText(char* szText, float64 fMinX, float64 fMinY, float64 fMaxX, float64 fMaxY, float64 fPixelsPerMillimetre, CGerberRaster* pcRaster)
{
	CGerberVectorImage	cImage;
	CGerberRasteriser	cRasteriser;
	SFloat64Vec2		sMinimum;
	SFloat64Vec2		sMaximum;

	cImage.Init();
	VectoriseGerberText(szText, &cImage);
	sMinimum.Init(fMinX, fMinY);
	sMaximum.Init(fMaxX, fMaxY);
	AssertTrue(pcRaster->Init(&sMinimum, &sMaximum, fPixelsPerMillimetre));
	cRasteriser.Init();
	AssertTrue(cRasteriser.Rasterise(&cImage, pcRaster));
	cRasteriser.Kill();
	cImage.Kill();
	return pcRaster->Area();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertRelative(float64 fExpected, float64 fActual, float64 fRelative)
{
	//Shows both values when they are too far apart.
	if (fabs(fActual - fExpected) <= fabs(fExpected) * fRelative)
	{
		AssertTrue(true);
	}
	else
	{
		AssertDouble(fExpected, fActual, 9);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberRasterRectangle(void)
{
	CGerberRaster	cRaster;
	int				x;
	int				y;
	int				iFull;
	int				iEmpty;

	//A 1 x 2 mm rectangle on the pixel grid at 100 pixels per millimetre is exactly 100 x 200 full pixels.
	RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10R,1X2*%\nD10*\nX1500000Y2000000D03*\n", 0, 0, 3, 4, 100, &cRaster);
	AssertInt(300, cRaster.GetWidth());
	AssertInt(400, cRaster.GetHeight());
	AssertDouble(2.0, cRaster.Area(), 9);
	iFull = 0;
	iEmpty = 0;
	for (y = 0; y < cRaster.GetHeight(); y++)
	{
		for (x = 0; x < cRaster.GetWidth(); x++)
		{
			if (cRaster.Get(x, y) == 255)
			{
				iFull++;
			}
			else if (cRaster.Get(x, y) == 0)
			{
				iEmpty++;
			}
		}
	}
	AssertInt(20000, iFull);
	AssertInt(300 * 400 - 20000, iEmpty);

	//The raster is y down; the rectangle spans y 1 to 3 mm so rows 100 to 299, columns 100 to 199.
	AssertInt(255, cRaster.Get(100, 100));
	AssertInt(255, cRaster.Get(199, 299));
	AssertInt(0, cRaster.Get(99, 100));
	AssertInt(0, cRaster.Get(100, 99));
	AssertInt(0, cRaster.Get(200, 299));
	AssertInt(0, cRaster.Get(199, 300));
	cRaster.Kill();

	//Half a pixel off the grid the edge pixels are half covered; 127.5 rounds to 128.
	RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10R,1X2*%\nD10*\nX1505000Y2000000D03*\n", 0, 0, 3, 4, 100, &cRaster);
	AssertDouble(2.0, cRaster.Area(), 3);
	AssertInt(128, cRaster.Get(100, 150));
	AssertInt(255, cRaster.Get(101, 150));
	AssertInt(128, cRaster.Get(200, 150));
	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberRasterCurves(void)
{
	CGerberRaster	cRaster;
	float64			fLens;

	//Circles; the flattened arcs keep the area.
	AssertRelative(N_PI_64, RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,2*%\nD10*\nX2000000Y2000000D03*\n", 0, 0, 4, 4, 40, &cRaster), 0.0002);
	cRaster.Kill();
	//A 4 pixel radius; the 8 bit coverage of its few edge pixels limits it to about 0.2%.
	AssertRelative(N_PI_64 * 0.16, RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,0.8*%\nD10*\nX2000000Y2000000D03*\n", 0, 0, 4, 4, 10, &cRaster), 0.005);
	cRaster.Kill();
	AssertRelative(N_PI_64 * 100, RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,20*%\nD10*\nX15000000Y15000000D03*\n", 0, 0, 30, 30, 20, &cRaster), 0.0001);
	cRaster.Kill();

	//A circle with a hole.
	AssertRelative(N_PI_64 * 0.75, RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,2X1*%\nD10*\nX2000000Y2000000D03*\n", 0, 0, 4, 4, 40, &cRaster), 0.0005);
	AssertInt(0, cRaster.Get(80, 80));
	AssertInt(255, cRaster.Get(80, 50));
	cRaster.Kill();

	//A draw and a quarter arc.
	AssertRelative(2 * 0.2 + N_PI_64 * 0.01, RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,0.2*%\nD10*\nG01*\nX1000000Y1000000D02*\nX3000000D01*\n", 0, 0, 4, 2, 40, &cRaster), 0.001);
	cRaster.Kill();
	AssertRelative(N_PI_64 * 0.11, RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,0.2*%\nD10*\nG75*\nG03*\nX2000000Y1000000D02*\nX1000000Y2000000I-1000000J0D01*\n", 0, 0, 3, 3, 40, &cRaster), 0.002);
	cRaster.Kill();

	//Two overlapping dark circles cover their union once.
	fLens = 2 * acos(0.5) - 0.5 * sqrt(3.0);
	AssertRelative(2 * N_PI_64 - fLens, RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,2*%\nD10*\nX2000000Y2000000D03*\nX3000000D03*\n", 0, 0, 5, 4, 40, &cRaster), 0.0005);
	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberRasterPolarity(void)
{
	CGerberRaster	cRaster;

	//A clear circle punches a hole in the rectangle before it and the dark circle after it covers the hole again.  Where
	//the edges of the hole and the circle meet their coverage is combined, so those pixels end a little short of dark.
	AssertRelative(8 - N_PI_64 * 0.25, RasteriseGerberText(
		"%FSLAX26Y26*%\n%MOMM*%\n%ADD10R,4X2*%\n%ADD11C,1*%\n"
		"D10*\nX3000000Y2000000D03*\n"
		"%LPC*%\nD11*\nX3000000Y2000000D03*\n", 0, 0, 6, 4, 40, &cRaster), 0.0005);
	AssertInt(0, cRaster.Get(120, 80));
	AssertInt(255, cRaster.Get(50, 80));
	cRaster.Kill();

	AssertRelative(8, RasteriseGerberText(
		"%FSLAX26Y26*%\n%MOMM*%\n%ADD10R,4X2*%\n%ADD11C,1*%\n"
		"D10*\nX3000000Y2000000D03*\n"
		"%LPC*%\nD11*\nX3000000Y2000000D03*\n"
		"%LPD*%\nX3000000Y2000000D03*\n", 0, 0, 6, 4, 40, &cRaster), 0.002);
	AssertInt(255, cRaster.Get(120, 80));
	cRaster.Kill();

	//A clear object over nothing does nothing.
	AssertDouble(0, RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD11C,1*%\n%LPC*%\nD11*\nX1000000Y1000000D03*\n", 0, 0, 2, 2, 40, &cRaster), 9);
	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberRasterMacros(void)
{
	CGerberRaster	cRaster;

	//Exposure off erases the earlier parts of the flash only: the donut hole shows the rectangle beneath it.
	AssertRelative(N_PI_64 * 0.75, RasteriseGerberText(
		"%FSLAX26Y26*%\n%MOMM*%\n%AMDONUT*1,1,2,0,0*1,0,1,0,0*%\n%ADD10DONUT*%\n"
		"D10*\nX2000000Y2000000D03*\n", 0, 0, 4, 4, 40, &cRaster), 0.0005);
	cRaster.Kill();
	AssertRelative(N_PI_64 * 0.75 + N_PI_64 * 0.25, RasteriseGerberText(
		"%FSLAX26Y26*%\n%MOMM*%\n%AMDONUT*1,1,2,0,0*1,0,1,0,0*%\n%ADD10DONUT*%\n%ADD11R,1X1*%\n"
		"D11*\nX2000000Y2000000D03*\n"
		"D10*\nX2000000Y2000000D03*\n", 0, 0, 4, 4, 40, &cRaster), 0.0005);
	AssertInt(255, cRaster.Get(80, 80));
	cRaster.Kill();

	//Primitives winding opposite ways still add: a clockwise square inside a circle.
	AssertRelative(N_PI_64, RasteriseGerberText(
		"%FSLAX26Y26*%\n%MOMM*%\n%AMMIXED*1,1,2,0,0*4,1,4,-0.5,-0.5,-0.5,0.5,0.5,0.5,0.5,-0.5,-0.5,-0.5,0*%\n%ADD10MIXED*%\n"
		"D10*\nX2000000Y2000000D03*\n", 0, 0, 4, 4, 40, &cRaster), 0.0005);
	AssertInt(255, cRaster.Get(80, 80));
	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberRasterClipping(void)
{
	CGerberRaster	cRaster;

	//Only the part inside the raster is drawn; edges left of it still count.
	AssertRelative(N_PI_64 / 2, RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,2*%\nD10*\nX0Y0D03*\n", 0, -2, 2, 2, 40, &cRaster), 0.0005);
	cRaster.Kill();
	AssertRelative(N_PI_64 / 4, RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,2*%\nD10*\nX0Y0D03*\n", -2, -2, 0, 0, 40, &cRaster), 0.0005);
	cRaster.Kill();
	AssertDouble(0, RasteriseGerberText("%FSLAX26Y26*%\n%MOMM*%\n%ADD10C,2*%\nD10*\nX10000000Y0D03*\n", -2, -2, 2, 2, 40, &cRaster), 9);
	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberRasterKiCadAdder16Bit(void)
{
	CExcellonCommands		cDrills;
	CExcellonInterpreter	cInterpreter;
	CExcellonVectoriser		cVectoriser;
	CGerberVectorImage		cImage;
	CGerberRasteriser		cRasteriser;
	CGerberRaster			cRaster;
	CGerberCommands			cCommands;
	SFloat64Vec2			sMinimum;
	SFloat64Vec2			sMaximum;
	float64					fExact;

	//83 holes of 0.39878 mm and 56 of 1.00076 mm.
	cDrills.Init();
	AssertTritrue(ParseExcellonFile("Adder 16bit" _FS_ "Adder 16bit-PTH.drl", &cDrills));
	cImage.Init();
	cVectoriser.Init(&cImage, true);
	cInterpreter.Init(&cDrills, &cVectoriser);
	AssertTrue(cInterpreter.Interpret());
	cInterpreter.Kill();
	cVectoriser.Kill();
	cDrills.Kill();

	AssertTrue(cImage.GetBounds(&sMinimum, &sMaximum));
	AssertTrue(cRaster.Init(&sMinimum, &sMaximum, 40));
	cRasteriser.Init();
	AssertTrue(cRasteriser.Rasterise(&cImage, &cRaster));
	cRasteriser.Kill();
	fExact = 83 * N_PI_64 * 0.19939 * 0.19939 + 56 * N_PI_64 * 0.50038 * 0.50038;
	AssertRelative(fExact, cRaster.Area(), 0.0002);
	cRaster.Kill();
	cImage.Kill();

	//The copper layer rasterises completely; its outline bounds its area.
	cCommands.Init();
	AssertTritrue(ParseGerberFile("KiCad" _FS_ "Adder 16bit", "Adder 16bit-F_Cu.gtl", &cCommands));
	cImage.Init();
	VectoriseGerberCommands(&cCommands, &cImage);
	AssertTrue(cImage.GetBounds(&sMinimum, &sMaximum));
	AssertTrue(cRaster.Init(&sMinimum, &sMaximum, 20));
	cRasteriser.Init();
	AssertTrue(cRasteriser.Rasterise(&cImage, &cRaster));
	cRasteriser.Kill();
	AssertTrue(cRaster.Area() > 300);
	AssertTrue(cRaster.Area() < (sMaximum.x - sMinimum.x) * (sMaximum.y - sMinimum.y));
	cRaster.Kill();
	cImage.Kill();
	cCommands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGerberRaster(void)
{
	TypeConverterInit();
	NumberInit();
	BeginTests();

	TestGerberRasterRectangle();
	TestGerberRasterCurves();
	TestGerberRasterPolarity();
	TestGerberRasterMacros();
	TestGerberRasterClipping();
	TestGerberRasterKiCadAdder16Bit();

	TestStatistics();
	NumberKill();
	TypeConverterKill();
}

