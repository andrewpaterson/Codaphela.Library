#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StdRandom.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/FlowContainer.h"
#include "WindowLib/ImageCanvasDraw.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "BorderCanvasDraw.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"
#include "DarkForest.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
struct SGridTestComponent
{
	const char*	szImage;			//NULL for a border canvas.
	int			iColumn;
	int			iRow;
	int			iColumnSpan;
	int			iRowSpan;
	int			iWidthWeight;
	int			iHeightWeight;
	SInt32Vec2	sDesiredSize;		//-1 on an axis fills the cell on that axis.  -1, -1 on an image canvas takes the cel's size.
	SInt32Vec2	sExpectedPosition;
	SInt32Vec2	sExpectedSize;

	EContainerStyleHorizontal	eHorizontal;	//Unknown on both axes leaves the canvas's default alignment.
	EContainerStyleVertical		eVertical;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerLayout(char* szDirectory, int32 iWidth, int32 iHeight, size uiNumComponents, SGridTestComponent* asComponents, uint32 iRandomSeed, int32 iExpectedRequiredWidth, int32 iExpectedRequiredHeight)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	CRandom					cRandom;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	cRandom.Init(iRandomSeed);

	ObjectsInit();
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CGridContainer>		pGrid;
		Ptr<CCanvas>			apCanvas[9];
		Ptr<CCanvasDraw>		pDraw;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		CColourFormatHelper		cHelper;
		SGridTestComponent*		psComponent;
		CChars					szExpectedDirectory;
		SInt32Vec2				sVec;
		CPointer				pNull;
		size					i;
		ARGB32					sColour;

		AssertTrue(uiNumComponents <= 9);

		cNativeFactory.Init(&gcMemoryAllocator, iWidth, iHeight, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Grid Container", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTestWindow->GetColourFormat(&cHelper);

		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);

		for (i = 0; i < uiNumComponents; i++)
		{
			psComponent = &asComponents[i];
			if (psComponent->szImage)
			{
				pDraw = OMalloc<CImageCanvasDraw>(ReadDarkForestCel(psComponent->szImage, &cHelper));
			}
			else
			{
				sColour = Set32BitColour(cRandom.Next(0, 255) / 255.0f, cRandom.Next(0, 255) / 255.0f, cRandom.Next(0, 255) / 255.0f);
				pDraw = OMalloc<CBorderCanvasDraw>(sColour);
			}
			apCanvas[i] = OMalloc<CCanvas>(pTestWindow, pDraw);
			apCanvas[i]->SetDesiredSize(psComponent->sDesiredSize.x, psComponent->sDesiredSize.y);
			if ((psComponent->eHorizontal != CSH_Unknown) || (psComponent->eVertical != CSV_Unknown))
			{
				apCanvas[i]->SetAlignment(psComponent->eHorizontal, psComponent->eVertical);
			}
			pGrid->AddComponent(apCanvas[i], psComponent->iColumn, psComponent->iRow, psComponent->iColumnSpan, psComponent->iRowSpan, psComponent->iWidthWeight, psComponent->iHeightWeight);
			pDraw = NULL;
		}

		pGrid->SetRequiredSize();
		sVec = pGrid->GetRequiredSize();
		AssertInt32Vec2(iExpectedRequiredWidth, iExpectedRequiredHeight, &sVec);

		pTestWindow->Show();

		for (i = 0; i < uiNumComponents; i++)
		{
			sVec = apCanvas[i]->GetPosition();
			AssertInt32Vec2(asComponents[i].sExpectedPosition.x, asComponents[i].sExpectedPosition.y, &sVec);
			sVec = apCanvas[i]->GetActualSize();
			AssertInt32Vec2(asComponents[i].sExpectedSize.x, asComponents[i].sExpectedSize.y, &sVec);
			apCanvas[i] = NULL;
		}

		pGrid = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");
		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cRandom.Kill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerNoWeights(void)
{
	//Column 0 is 14 wide and column 1 is 6 wide, row 0 is 12 tall and row 1 is 5 tall.  With no weights nothing takes the
	//spare space so the 20 x 17 grid is centred in the 40 x 30 window, and each component is centred in its cell.
	SGridTestComponent	asComponents[4] = {
		{ NULL, 0, 0, 1, 1, 0, 0, SInt32Vec2(10, 8),  SInt32Vec2(12, 8),  SInt32Vec2(10, 8), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 0, 0, SInt32Vec2(6, 12),  SInt32Vec2(24, 6),  SInt32Vec2(6, 12), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 1, 1, 0, 0, SInt32Vec2(14, 5),  SInt32Vec2(10, 18), SInt32Vec2(14, 5), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 1, 1, 1, 0, 0, SInt32Vec2(4, 4),   SInt32Vec2(25, 18), SInt32Vec2(4, 4), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerNoWeights", 40, 30, 4, asComponents, 4354, 20, 17);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerColumnWeights(void)
{
	//The 60 spare pixels are shared 1 : 2 so column 0 grows from 10 to 30 and column 1 from 0 to 40.  The fixed canvas is
	//centred in its cell and the canvas without a desired size fills its cell.
	SGridTestComponent	asComponents[2] = {
		{ NULL, 0, 0, 1, 1, 1, 0, SInt32Vec2(10, 10), SInt32Vec2(10, 0),  SInt32Vec2(10, 10), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 2, 0, SInt32Vec2(-1, -1), SInt32Vec2(30, 0),  SInt32Vec2(40, 10), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerColumnWeights", 70, 10, 2, asComponents, 4674, 10, 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerRowWeights(void)
{
	//The 22 spare pixels are shared evenly between the two rows, 8 + 11 and 0 + 11.
	SGridTestComponent	asComponents[2] = {
		{ NULL, 0, 0, 1, 1, 0, 1, SInt32Vec2(8, 8),   SInt32Vec2(0, 5),   SInt32Vec2(8, 8), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 1, 1, 0, 1, SInt32Vec2(8, -1),  SInt32Vec2(0, 19),  SInt32Vec2(8, 11), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerRowWeights", 8, 30, 2, asComponents, 9844, 8, 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerSpanWithoutWeights(void)
{
	//The 20 wide component spans the two 6 wide columns.  With no weights the missing 8 pixels go to the last column.
	SGridTestComponent	asComponents[3] = {
		{ NULL, 0, 0, 1, 1, 0, 0, SInt32Vec2(6, 4),   SInt32Vec2(0, 0),   SInt32Vec2(6, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 0, 0, SInt32Vec2(6, 4),   SInt32Vec2(10, 0),  SInt32Vec2(6, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 2, 1, 0, 0, SInt32Vec2(20, 4),  SInt32Vec2(0, 4),   SInt32Vec2(20, 4), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerSpanWithoutWeights", 20, 8, 3, asComponents, 2673, 20, 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerSpanWithWeights(void)
{
	//With column weights of 1 and 3 the missing 8 pixels are shared 2 and 6, making the columns 8 and 12 wide.
	SGridTestComponent	asComponents[3] = {
		{ NULL, 0, 0, 1, 1, 1, 0, SInt32Vec2(6, 4),   SInt32Vec2(1, 0),   SInt32Vec2(6, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 3, 0, SInt32Vec2(6, 4),   SInt32Vec2(11, 0),  SInt32Vec2(6, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 2, 1, 0, 0, SInt32Vec2(20, 4),  SInt32Vec2(0, 4),   SInt32Vec2(20, 4), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerSpanWithWeights", 20, 8, 3, asComponents, 1234, 20, 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerShrink(void)
{
	//The two 20 wide columns need 40 pixels.  In 30 pixels the weighted column gives up the missing 10, leaving its fixed
	//component centred over a 10 wide cell.
	SGridTestComponent	asWeightedFirst[2] = {
		{ NULL, 0, 0, 1, 1, 0, 0, SInt32Vec2(20, 4),  SInt32Vec2(0, 0),   SInt32Vec2(20, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 1, 0, SInt32Vec2(20, 4),  SInt32Vec2(15, 0),  SInt32Vec2(20, 4), CSH_Unknown, CSV_Unknown } };

	//In 10 pixels the weighted column gives up all 20 of its pixels and the other column then shrinks to 10.
	SGridTestComponent	asThenEverything[2] = {
		{ NULL, 0, 0, 1, 1, 0, 0, SInt32Vec2(20, 4),  SInt32Vec2(-5, 0),  SInt32Vec2(20, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 1, 0, SInt32Vec2(20, 4),  SInt32Vec2(0, 0),   SInt32Vec2(20, 4), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerShrinkWeightedFirst", 30, 4, 2, asWeightedFirst, 6665, 40, 4);
	TestGridContainerLayout("Output" _FS_ "GridContainerShrinkThenEverything", 10, 4, 2, asThenEverything, 6665, 40, 4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerOverlap(void)
{
	//Two components in the same cell are both laid out in it.
	SGridTestComponent	asComponents[2] = {
		{ NULL, 0, 0, 1, 1, 0, 0, SInt32Vec2(10, 10), SInt32Vec2(0, 0),   SInt32Vec2(10, 10), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 0, 1, 1, 0, 0, SInt32Vec2(4, 4),   SInt32Vec2(3, 3),   SInt32Vec2(4, 4), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerOverlap", 10, 10, 2, asComponents, 7936, 10, 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerGrowSlivers(void)
{
	//The fixed components in row 0 set the columns and the components in row 1 fill them to show how wide each is.

	//61 spare pixels shared 1 : 2 is 20 and 40 with 1 left over.  It goes to the larger column, 50 wide, not the first.
	SGridTestComponent	asLargest[4] = {
		{ NULL, 0, 0, 1, 1, 1, 0, SInt32Vec2(10, 4), SInt32Vec2(10, 0), SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 2, 0, SInt32Vec2(10, 4), SInt32Vec2(50, 0), SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(0, 4), SInt32Vec2(30, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(30, 4), SInt32Vec2(51, 4), CSH_Unknown, CSV_Unknown } };

	//5 spare pixels between three equal columns is 1 each with 2 left over, which go to the first two.
	SGridTestComponent	asTies[6] = {
		{ NULL, 0, 0, 1, 1, 1, 0, SInt32Vec2(10, 4), SInt32Vec2(1, 0), SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 1, 0, SInt32Vec2(10, 4), SInt32Vec2(13, 0), SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 2, 0, 1, 1, 1, 0, SInt32Vec2(10, 4), SInt32Vec2(24, 0), SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(0, 4), SInt32Vec2(12, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(12, 4), SInt32Vec2(12, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 2, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(24, 4), SInt32Vec2(11, 4), CSH_Unknown, CSV_Unknown } };

	//4 spare pixels between columns of 5, 20 and 10 is 1 each with 1 left over, which goes to the middle, largest, column.
	SGridTestComponent	asLargestInMiddle[6] = {
		{ NULL, 0, 0, 1, 1, 1, 0, SInt32Vec2(5, 4), SInt32Vec2(0, 0), SInt32Vec2(5, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 1, 0, SInt32Vec2(20, 4), SInt32Vec2(7, 0), SInt32Vec2(20, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 2, 0, 1, 1, 1, 0, SInt32Vec2(10, 4), SInt32Vec2(28, 0), SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(0, 4), SInt32Vec2(6, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(6, 4), SInt32Vec2(22, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 2, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(28, 4), SInt32Vec2(11, 4), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerGrowSliversLargest", 81, 8, 4, asLargest, 7740, 20, 8);
	TestGridContainerLayout("Output" _FS_ "GridContainerGrowSliversTies", 35, 8, 6, asTies, 7740, 30, 8);
	TestGridContainerLayout("Output" _FS_ "GridContainerGrowSliversLargestInMiddle", 39, 8, 6, asLargestInMiddle, 7740, 35, 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerSpanSlivers(void)
{
	//The 19 wide span is 7 more than its columns of 8 and 4.  Shared 1 : 1 that is 3 each with 1 left over, which goes to
	//the larger column, making them 12 and 7.  Row 2 shows the column widths.
	SGridTestComponent	asComponents[5] = {
		{ NULL, 0, 0, 1, 1, 1, 0, SInt32Vec2(8, 4), SInt32Vec2(2, 0), SInt32Vec2(8, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 1, 0, SInt32Vec2(4, 4), SInt32Vec2(13, 0), SInt32Vec2(4, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 2, 1, 0, 0, SInt32Vec2(19, 4), SInt32Vec2(0, 4), SInt32Vec2(19, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 2, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(0, 8), SInt32Vec2(12, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 2, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(12, 8), SInt32Vec2(7, 4), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerSpanSlivers", 19, 12, 5, asComponents, 2345, 19, 12);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerShrinkSlivers(void)
{
	//Columns of 10 and 20, both weighted, 1 pixel too wide.  Shared 1 : 1 each would give 0, so the larger column gives it.
	SGridTestComponent	asOnePixel[4] = {
		{ NULL, 0, 0, 1, 1, 1, 0, SInt32Vec2(10, 4), SInt32Vec2(0, 0), SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 1, 0, SInt32Vec2(20, 4), SInt32Vec2(10, 0), SInt32Vec2(20, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(0, 4), SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(10, 4), SInt32Vec2(19, 4), CSH_Unknown, CSV_Unknown } };

	//5 pixels too wide.  Each gives 2 and the larger gives the 1 left over, leaving 8 and 17.
	SGridTestComponent	asFivePixels[4] = {
		{ NULL, 0, 0, 1, 1, 1, 0, SInt32Vec2(10, 4), SInt32Vec2(-1, 0), SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 1, 0, SInt32Vec2(20, 4), SInt32Vec2(7, 0), SInt32Vec2(20, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(0, 4), SInt32Vec2(8, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(8, 4), SInt32Vec2(17, 4), CSH_Unknown, CSV_Unknown } };

	//Unweighted columns of 30, 20 and 10 are 7 pixels too wide.  In proportion they give 3, 2 and 1, and the largest gives
	//the 1 left over, leaving 26, 18 and 9.
	SGridTestComponent	asProportional[6] = {
		{ NULL, 0, 0, 1, 1, 0, 0, SInt32Vec2(30, 4), SInt32Vec2(-2, 0), SInt32Vec2(30, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 0, 0, SInt32Vec2(20, 4), SInt32Vec2(25, 0), SInt32Vec2(20, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 2, 0, 1, 1, 0, 0, SInt32Vec2(10, 4), SInt32Vec2(44, 0), SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(0, 4), SInt32Vec2(26, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(26, 4), SInt32Vec2(18, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 2, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4), SInt32Vec2(44, 4), SInt32Vec2(9, 4), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerShrinkSliversOnePixel", 29, 8, 4, asOnePixel, 8489, 30, 8);
	TestGridContainerLayout("Output" _FS_ "GridContainerShrinkSliversFivePixels", 25, 8, 4, asFivePixels, 8489, 30, 8);
	TestGridContainerLayout("Output" _FS_ "GridContainerShrinkSliversProportional", 53, 8, 6, asProportional, 8489, 60, 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerEmpty(void)
{
	//A grid with nothing in it needs no space and lays out without anything to lay out.
	TestGridContainerLayout("Output" _FS_ "GridContainerEmpty", 20, 20, 0, NULL, 7735, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerGaps(void)
{
	//Column 1 has nothing in it so it is 0 wide and the component in column 2 sits right next to the one in column 0.
	SGridTestComponent	asEmptyColumn[2] = {
		{ NULL, 0, 0, 1, 1, 0, 0, SInt32Vec2(6, 6),   SInt32Vec2(0, 0),   SInt32Vec2(6, 6), CSH_Unknown, CSV_Unknown },
		{ NULL, 2, 0, 1, 1, 0, 0, SInt32Vec2(6, 6),   SInt32Vec2(6, 0),   SInt32Vec2(6, 6), CSH_Unknown, CSV_Unknown } };

	//On its own in cell (2, 1) the empty columns and row take no space, so the 10 x 10 grid is centred in the window.
	SGridTestComponent	asAlone[1] = {
		{ NULL, 2, 1, 1, 1, 0, 0, SInt32Vec2(10, 10), SInt32Vec2(5, 5),   SInt32Vec2(10, 10), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerGapsEmptyColumn", 12, 6, 2, asEmptyColumn, 6112, 12, 6);
	TestGridContainerLayout("Output" _FS_ "GridContainerGapsAlone", 20, 20, 1, asAlone, 6112, 10, 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerImageInWeightedCell(void)
{
	//An image canvas takes the size of its cel, so even in a cell weighted to take the whole window it keeps that size
	//and is centred.  A border canvas with no desired size fills the same cell.
	SGridTestComponent	asImage[1] = {
		{ "Wolf.png", 0, 0, 1, 1, 1, 1, SInt32Vec2(-1, -1), SInt32Vec2(9, 5),   SInt32Vec2(12, 10), CSH_Unknown, CSV_Unknown } };
	SGridTestComponent	asBorder[1] = {
		{ NULL,       0, 0, 1, 1, 1, 1, SInt32Vec2(-1, -1), SInt32Vec2(0, 0),   SInt32Vec2(30, 20), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerImageInWeightedCell", 30, 20, 1, asImage, 6551, 12, 10);
	TestGridContainerLayout("Output" _FS_ "GridContainerBorderInWeightedCell", 30, 20, 1, asBorder, 6551, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerDarkForestImages(void)
{
	//Three images on the top row and the wolf spanning the first two columns of the bottom row.  The columns are 9, 9 and
	//10 wide and the rows 14 and 10 tall, so the 28 x 24 grid is centred at 6, 3 in the 40 x 30 window.
	SGridTestComponent	asComponents[5] = {
		{ "Pine.png",		0, 0, 1, 1, 0, 0, SInt32Vec2(-1, -1), SInt32Vec2(6, 3),   SInt32Vec2(9, 14), CSH_Unknown, CSV_Unknown },
		{ "Owl.png",		1, 0, 1, 1, 0, 0, SInt32Vec2(-1, -1), SInt32Vec2(15, 5),  SInt32Vec2(9, 10), CSH_Unknown, CSV_Unknown },
		{ "Moon.png",		2, 0, 1, 1, 0, 0, SInt32Vec2(-1, -1), SInt32Vec2(24, 5),  SInt32Vec2(10, 10), CSH_Unknown, CSV_Unknown },
		{ "Wolf.png",		0, 1, 2, 1, 0, 0, SInt32Vec2(-1, -1), SInt32Vec2(9, 17),  SInt32Vec2(12, 10), CSH_Unknown, CSV_Unknown },
		{ "Mushroom.png",	2, 1, 1, 1, 0, 0, SInt32Vec2(-1, -1), SInt32Vec2(25, 18), SInt32Vec2(8, 8), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerDarkForestImages", 40, 30, 5, asComponents, 6510, 28, 24);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerComponentAlignment(void)
{
	//Every column and row has a component 30 pixels across so the 90 x 90 grid is evenly weighted.  The 60 spare pixels
	//make the columns 50 wide and the 30 spare pixels make the rows 40 tall.  Each component is placed in its 50 x 40
	//cell by its own alignment: the corners and edges by left, right, top and bottom, stretched components fill their
	//cell on that axis and centred ones sit in the middle.
	SGridTestComponent	asComponents[9] = {
		{ NULL,			0, 0, 1, 1, 1, 1, SInt32Vec2(30, 30), SInt32Vec2(0, 0),     SInt32Vec2(30, 30), CSH_Left,		CSV_Top },
		{ "Owl.png",	1, 0, 1, 1, 1, 1, SInt32Vec2(-1, -1), SInt32Vec2(70, 0),    SInt32Vec2(9, 10),  CSH_Center,		CSV_Top },
		{ NULL,			2, 0, 1, 1, 1, 1, SInt32Vec2(24, 18), SInt32Vec2(126, 0),   SInt32Vec2(24, 18), CSH_Right,		CSV_Top },
		{ "Wolf.png",	0, 1, 1, 1, 1, 1, SInt32Vec2(-1, -1), SInt32Vec2(0, 55),    SInt32Vec2(12, 10), CSH_Left,		CSV_Center },
		{ NULL,			1, 1, 1, 1, 1, 1, SInt32Vec2(30, 30), SInt32Vec2(50, 45),   SInt32Vec2(50, 30), CSH_Stretch,	CSV_Center },
		{ "Pine.png",	2, 1, 1, 1, 1, 1, SInt32Vec2(-1, -1), SInt32Vec2(141, 53),  SInt32Vec2(9, 14),  CSH_Right,		CSV_Center },
		{ "Moon.png",	0, 2, 1, 1, 1, 1, SInt32Vec2(-1, -1), SInt32Vec2(0, 110),   SInt32Vec2(10, 10), CSH_Left,		CSV_Bottom },
		{ NULL,			1, 2, 1, 1, 1, 1, SInt32Vec2(16, 30), SInt32Vec2(67, 80),   SInt32Vec2(16, 40), CSH_Center,		CSV_Stretch },
		{ NULL,			2, 2, 1, 1, 1, 1, SInt32Vec2(30, 12), SInt32Vec2(120, 108), SInt32Vec2(30, 12), CSH_Right,		CSV_Bottom } };

	TestGridContainerLayout("Output" _FS_ "GridContainerComponentAlignment", 150, 120, 9, asComponents, 3821, 90, 90);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerRowSpans(void)
{
	//The 20 tall component spans the two 6 tall rows.  With no weights the missing 8 pixels go to the last row.
	SGridTestComponent	asWithoutWeights[3] = {
		{ NULL, 0, 0, 1, 1, 0, 0, SInt32Vec2(4, 6),   SInt32Vec2(0, 0),   SInt32Vec2(4, 6), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 1, 1, 0, 0, SInt32Vec2(4, 6),   SInt32Vec2(0, 10),  SInt32Vec2(4, 6), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 2, 0, 0, SInt32Vec2(4, 20),  SInt32Vec2(4, 0),   SInt32Vec2(4, 20), CSH_Unknown, CSV_Unknown } };

	//With row weights of 1 and 3 the missing 8 pixels are shared 2 and 6, making the rows 8 and 12 tall.
	SGridTestComponent	asWithWeights[3] = {
		{ NULL, 0, 0, 1, 1, 0, 1, SInt32Vec2(4, 6),   SInt32Vec2(0, 1),   SInt32Vec2(4, 6), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 1, 1, 0, 3, SInt32Vec2(4, 6),   SInt32Vec2(0, 11),  SInt32Vec2(4, 6), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 2, 0, 0, SInt32Vec2(4, 20),  SInt32Vec2(4, 0),   SInt32Vec2(4, 20), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerRowSpansWithoutWeights", 8, 20, 3, asWithoutWeights, 4510, 8, 20);
	TestGridContainerLayout("Output" _FS_ "GridContainerRowSpansWithWeights", 8, 20, 3, asWithWeights, 4510, 8, 20);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerSpanWeight(void)
{
	//A spanning cell's weight is given to the columns it spans when they don't already have that much.  Neither spanned
	//column has a weight, so the weight goes to the last of them, which then takes all 20 spare pixels.  The spanning
	//component fills its cell.
	SGridTestComponent	asToLastColumn[3] = {
		{ NULL, 0, 0, 1, 1, 0, 0, SInt32Vec2(10, 4),  SInt32Vec2(0, 0),   SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 0, 0, SInt32Vec2(10, 4),  SInt32Vec2(20, 0),  SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 2, 1, 1, 0, SInt32Vec2(-1, 4),  SInt32Vec2(0, 4),   SInt32Vec2(40, 4), CSH_Unknown, CSV_Unknown } };

	//Columns 0 and 1 have weights 1 and 3 and the cell spanning them has weight 8, so the missing 4 is shared 1 : 3 making
	//them 2 and 6.  Column 2 keeps weight 4.  The 24 spare pixels are then shared 2 : 6 : 4, so 4, 12 and 8.  Without the
	//spanning weight they would have been shared 1 : 3 : 4.
	SGridTestComponent	asToWeightedColumns[4] = {
		{ NULL, 0, 0, 1, 1, 1, 0, SInt32Vec2(10, 4),  SInt32Vec2(2, 0),   SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 0, 1, 1, 3, 0, SInt32Vec2(10, 4),  SInt32Vec2(20, 0),  SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 2, 0, 1, 1, 4, 0, SInt32Vec2(10, 4),  SInt32Vec2(40, 0),  SInt32Vec2(10, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 2, 1, 8, 0, SInt32Vec2(-1, 4),  SInt32Vec2(0, 4),   SInt32Vec2(36, 4), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerSpanWeightToLastColumn", 40, 8, 3, asToLastColumn, 6589, 20, 8);
	TestGridContainerLayout("Output" _FS_ "GridContainerSpanWeightToWeightedColumns", 54, 8, 4, asToWeightedColumns, 6589, 30, 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerNestedSpans(void)
{
	//The 3 column span is added before the 2 column span, but narrower spans are settled first.  The 2 column span puts
	//its missing 12 pixels into column 1, its last column, then the 3 column span finds 12 of its 30 and puts the other
	//18 into column 2.  Settled the other way round column 2 would be 30 wide.  The last component fills column 2.
	SGridTestComponent	asComponents[3] = {
		{ NULL, 0, 0, 3, 1, 0, 0, SInt32Vec2(30, 4),  SInt32Vec2(0, 0),   SInt32Vec2(30, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 0, 1, 2, 1, 0, 0, SInt32Vec2(12, 4),  SInt32Vec2(0, 4),   SInt32Vec2(12, 4), CSH_Unknown, CSV_Unknown },
		{ NULL, 2, 1, 1, 1, 0, 0, SInt32Vec2(-1, 4),  SInt32Vec2(12, 4),  SInt32Vec2(18, 4), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerNestedSpans", 30, 8, 3, asComponents, 6522, 30, 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerClamping(void)
{
	//A negative column or row is treated as 0, a span below 1 as 1 and a negative weight as 0.  So the first component is
	//an ordinary unweighted cell (0, 0) and the two 10 x 10 cells exactly fill the 20 x 20 window.
	SGridTestComponent	asComponents[2] = {
		{ NULL, -2, -1, 0, -3, -5, -1, SInt32Vec2(10, 10), SInt32Vec2(0, 0),   SInt32Vec2(10, 10), CSH_Unknown, CSV_Unknown },
		{ NULL, 1, 1, 1, 1, 0, 0,      SInt32Vec2(10, 10), SInt32Vec2(10, 10), SInt32Vec2(10, 10), CSH_Unknown, CSV_Unknown } };

	TestGridContainerLayout("Output" _FS_ "GridContainerClamping", 20, 20, 2, asComponents, 6581, 20, 20);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCanvas> CreateGridTestCanvas(Ptr<CWindow> pWindow, int32 iWidth, int32 iHeight)
{
	Ptr<CCanvas>	pCanvas;

	pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0)));
	pCanvas->SetDesiredSize(iWidth, iHeight);
	return pCanvas;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerAddAndRemove(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "GridContainerAddAndRemove";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CGridContainer>		pGrid;
		Ptr<CComponent>			pGridAsComponent;
		Ptr<CCanvas>			pCanvas0;
		Ptr<CCanvas>			pCanvas1;
		Ptr<CCanvas>			pCanvas2;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		SLogConfig				sLogConfig;
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 20, 20, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Grid Container", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);

		//A component can only be added to a grid with a cell, even through a pointer to it as a plain component.
		pCanvas0 = CreateGridTestCanvas(pTestWindow, 10, 10);
		pGridAsComponent = pGrid;
		sLogConfig = gcLogger.SetSilent();
		AssertFalse(pGrid->AddComponent(pCanvas0));
		AssertFalse(pGridAsComponent->AddComponent(pCanvas0));
		gcLogger.SetConfig(&sLogConfig);
		AssertSize(0, pGrid->NumComponents());

		//Three 10 x 10 components in cells (0, 0), (1, 0) and (0, 1).  Removing the middle one takes its cell with it, so the
		//others stay in their own cells: one column and two rows make a 10 x 20 grid centred in the 20 x 20 window.  If the
		//cell were left behind the last component would take cell (1, 0) and the grid would be 20 x 10.
		pCanvas1 = CreateGridTestCanvas(pTestWindow, 10, 10);
		pCanvas2 = CreateGridTestCanvas(pTestWindow, 10, 10);
		AssertTrue(pGrid->AddComponent(pCanvas0, 0, 0));
		AssertTrue(pGrid->AddComponent(pCanvas1, 1, 0));
		AssertTrue(pGrid->AddComponent(pCanvas2, 0, 1));
		AssertSize(3, pGrid->NumComponents());

		AssertTrue(pGridAsComponent->RemoveComponent(pCanvas1));
		AssertSize(2, pGrid->NumComponents());
		AssertFalse(pGrid->RemoveComponent(pCanvas1));

		pTestWindow->Show();
		sVec = pCanvas0->GetPosition();
		AssertInt32Vec2(5, 0, &sVec);
		sVec = pCanvas2->GetPosition();
		AssertInt32Vec2(5, 10, &sVec);

		//Removing everything removes every cell, so a component added afterwards gets its own cell.  A component that fills a
		//weighted cell (1, 1) takes the whole window.  Had the old cells been left behind it would have the empty unweighted
		//cell (0, 0) instead and be 0 x 0 in the middle of the window.
		AssertTrue(pGrid->RemoveAllComponents());
		AssertSize(0, pGrid->NumComponents());
		pCanvas1->SetDesiredSize(-1, -1);
		AssertTrue(pGrid->AddComponent(pCanvas1, 1, 1, 1, 1, 1, 1));
		pTestWindow->Layout();
		sVec = pCanvas1->GetPosition();
		AssertInt32Vec2(0, 0, &sVec);
		sVec = pCanvas1->GetActualSize();
		AssertInt32Vec2(20, 20, &sVec);

		pCanvas0 = NULL;
		pCanvas1 = NULL;
		pCanvas2 = NULL;
		pGridAsComponent = NULL;
		pGrid = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerNotAtOrigin(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "GridContainerNotAtOrigin";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFillContainer>		pFill;
		Ptr<CGridContainer>		pGrid;
		Ptr<CCanvas>			pFixed;
		Ptr<CCanvas>			pFilling;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		SInsets					sInsets;
		int						aiInsets[4] = { 3, 0, 0, 5 };  //Top, Right, Bottom, Left.
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 30, 20, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Grid Container", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		//The grid is inside a fill container with insets, so it is laid out in the 25 x 17 area at 5, 3 and every cell must
		//be offset by that.  A fixed 10 x 10 cell and a weighted cell that fills the rest of the width and the full height.
		pFill = OMalloc<CFillContainer>(pTestWindow);
		sInsets.Init(aiInsets);
		pFill->SetInsets(&sInsets);
		pTestWindow->SetContainer(pFill);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pFill->AddComponent(pGrid);

		pFixed = CreateGridTestCanvas(pTestWindow, 10, 10);
		pFilling = CreateGridTestCanvas(pTestWindow, -1, -1);
		AssertTrue(pGrid->AddComponent(pFixed, 0, 0));
		AssertTrue(pGrid->AddComponent(pFilling, 1, 0, 1, 1, 1, 1));

		pTestWindow->Show();

		sVec = pFixed->GetPosition();
		AssertInt32Vec2(5, 6, &sVec);
		sVec = pFixed->GetActualSize();
		AssertInt32Vec2(10, 10, &sVec);
		sVec = pFilling->GetPosition();
		AssertInt32Vec2(15, 3, &sVec);
		sVec = pFilling->GetActualSize();
		AssertInt32Vec2(15, 17, &sVec);

		pFixed = NULL;
		pFilling = NULL;
		pGrid = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertContainerBounds(int32 iLeft, int32 iTop, int32 iRight, int32 iBottom, CContainer* pcContainer)
{
	CInt32Rect	cBounds;

	AssertTrue(pcContainer->GetContainerBounds(&cBounds));
	AssertInt(iLeft, cBounds.GetLeft());
	AssertInt(iTop, cBounds.GetTop());
	AssertInt(iRight, cBounds.GetRight());
	AssertInt(iBottom, cBounds.GetBottom());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerActualBounds(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "GridContainerActualBounds";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CFillContainer>		pFill;
		Ptr<CGridContainer>		pGrid;
		Ptr<CFlowContainer>		pFlow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		SInsets					sInsets;
		int						aiInsets[4] = { 3, 0, 0, 5 };  //Top, Right, Bottom, Left.
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 30, 20, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Grid Container", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		//A container's actual bounds are the area its parent gave it, insets included and not clipped.  Its container
		//bounds are the inset area its components are laid out in, clipped to its parent's container bounds.  The grid is
		//40 x 10 at the top left of the fill's 25 x 17 inset area at 5, 3 so it hangs 15 pixels out of the right side.  The
		//flow has no desired height and sits along the bottom of the inset area.
		pFill = OMalloc<CFillContainer>(pTestWindow);
		sInsets.Init(aiInsets);
		pFill->SetInsets(&sInsets);
		pTestWindow->SetContainer(pFill);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pGrid->SetDesiredSize(40, 10);
		pGrid->SetAlignment(CSH_Left, CSV_Top);
		pFill->AddComponent(pGrid);
		pFlow = OMalloc<CFlowContainer>(pTestWindow);
		pFlow->SetDesiredSize(-1, 4);
		pFlow->SetAlignment(CSH_Center, CSV_Bottom);
		pFill->AddComponent(pFlow);

		pTestWindow->Show();

		sVec = pFill->GetPosition();
		AssertInt32Vec2(0, 0, &sVec);
		sVec = pFill->GetActualSize();
		AssertInt32Vec2(30, 20, &sVec);
		AssertContainerBounds(5, 3, 30, 20, &pFill);

		sVec = pGrid->GetPosition();
		AssertInt32Vec2(5, 3, &sVec);
		sVec = pGrid->GetActualSize();
		AssertInt32Vec2(40, 10, &sVec);
		AssertContainerBounds(5, 3, 30, 13, &pGrid);

		sVec = pFlow->GetPosition();
		AssertInt32Vec2(5, 16, &sVec);
		sVec = pFlow->GetActualSize();
		AssertInt32Vec2(25, 4, &sVec);
		AssertContainerBounds(5, 16, 30, 20, &pFlow);

		pFlow = NULL;
		pGrid = NULL;
		pFill = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainerRelayout(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "GridContainerRelayout";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CGridContainer>		pGrid;
		Ptr<CCanvas>			pA;
		Ptr<CCanvas>			pB;
		Ptr<CCanvas>			pC;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 40, 10, szDirectory);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Grid Container", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);

		//Each layout must come only from the grid as it is now, nothing left over from the layout before.
		pA = CreateGridTestCanvas(pTestWindow, 10, 10);
		pB = CreateGridTestCanvas(pTestWindow, 10, 10);
		AssertTrue(pGrid->AddComponent(pA, 0, 0));
		AssertTrue(pGrid->AddComponent(pB, 1, 0));
		pTestWindow->Show();
		sVec = pA->GetPosition();
		AssertInt32Vec2(10, 0, &sVec);  //The 20 wide grid centred in 40.
		sVec = pB->GetPosition();
		AssertInt32Vec2(20, 0, &sVec);

		//B grows, so its column does too and the 30 wide grid is centred at 5.
		pB->SetDesiredSize(20, 10);
		pTestWindow->Layout();
		sVec = pA->GetPosition();
		AssertInt32Vec2(5, 0, &sVec);
		sVec = pB->GetPosition();
		AssertInt32Vec2(15, 0, &sVec);
		sVec = pB->GetActualSize();
		AssertInt32Vec2(20, 10, &sVec);

		//A 4 x 4 component in a new column 2 makes the grid 34 wide, centred at 3.  It is centred in the 10 tall row.
		pC = CreateGridTestCanvas(pTestWindow, 4, 4);
		AssertTrue(pGrid->AddComponent(pC, 2, 0));
		pTestWindow->Layout();
		sVec = pA->GetPosition();
		AssertInt32Vec2(3, 0, &sVec);
		sVec = pB->GetPosition();
		AssertInt32Vec2(13, 0, &sVec);
		sVec = pC->GetPosition();
		AssertInt32Vec2(33, 3, &sVec);

		//Removing B empties column 1, so the grid is 14 wide and centred at 13.
		AssertTrue(pGrid->RemoveComponent(pB));
		pTestWindow->Layout();
		sVec = pA->GetPosition();
		AssertInt32Vec2(13, 0, &sVec);
		sVec = pC->GetPosition();
		AssertInt32Vec2(23, 3, &sVec);

		pA = NULL;
		pB = NULL;
		pC = NULL;
		pGrid = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestGridContainer(void)
{
	BeginTests();

	TestGridContainerNoWeights();
	TestGridContainerColumnWeights();
	TestGridContainerRowWeights();
	TestGridContainerSpanWithoutWeights();
	TestGridContainerSpanWithWeights();
	TestGridContainerRowSpans();
	TestGridContainerSpanWeight();
	TestGridContainerNestedSpans();
	TestGridContainerClamping();
	TestGridContainerEmpty();
	TestGridContainerGaps();
	TestGridContainerImageInWeightedCell();
	TestGridContainerGrowSlivers();
	TestGridContainerSpanSlivers();
	TestGridContainerShrinkSlivers();
	TestGridContainerShrink();
	TestGridContainerOverlap();
	TestGridContainerDarkForestImages();
	TestGridContainerComponentAlignment();
	TestGridContainerAddAndRemove();
	TestGridContainerNotAtOrigin();
	TestGridContainerActualBounds();
	TestGridContainerRelayout();

	TestStatistics();
}

