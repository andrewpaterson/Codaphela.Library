#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/StdRandom.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/FlowContainer.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "TestRefWindowCanvasDraw.h"
#include "BorderCanvasDraw.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"
#include "DarkForest.h"


bool gbAssertFile = true;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertImageFile(CChars* pszExpected, CChars* pszActual)
{
	CFileUtil	cFileUtil;
	bool		bSuccess;

	if (gbAssertFile)
	{
		AssertFile(pszExpected->Text(), pszActual->Text());
	}
	else
	{
		bSuccess = cFileUtil.Delete(pszExpected->Text());
		AssertTrue(bSuccess);
		bSuccess = cFileUtil.Copy(pszActual->Text(), pszExpected->Text());
		AssertTrue(bSuccess);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertFlowContainerObjectsRemoved(void)
{
	//Every free Object must have been removed by its owner.  Removing all from the Root then also removes FreeObjects and its Set leaving only the Root and its Set.
	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerBasicOnHeap(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "FlowContainerBasic";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		CChars					szExpectedDirectory;
		Ptr<CCanvas>			pCanvas1;
		Ptr<CCanvas>			pCanvas2;
		Ptr<CCanvas>			pCanvas3;
		Ptr<CBorderCanvasDraw>	pDraw1;
		Ptr<CBorderCanvasDraw>	pDraw2;
		Ptr<CBorderCanvasDraw>	pDraw3;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		Ptr<CFlowContainer>		pFlow;
		CInt32Rect				cBounds;
		SInt32Vec2				sVec;

		cNativeFactory.Init(&gcMemoryAllocator, 96, 30, szDirectory);

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, (CPointer)NULL, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pDraw1 = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0));
		pCanvas1 = OMalloc<CCanvas>(pTestWindow, pDraw1);
		pDraw2 = OMalloc<CBorderCanvasDraw>(Set32BitColour(0, 1.0f, 0));
		pCanvas2 = OMalloc<CCanvas>(pTestWindow, pDraw2);
		pDraw3 = OMalloc<CBorderCanvasDraw>(Set32BitColour(0, 0, 1.0f));
		pCanvas3 = OMalloc<CCanvas>(pTestWindow, pDraw3);

		pFlow = OMalloc<CFlowContainer>(pTestWindow);
		pFlow->SetFlowStyle(CSD_Right, CSW_Continue, CSV_Top);
		pTestWindow->SetContainer(pFlow);

		pFlow->AddComponent(pCanvas1);
		pFlow->AddComponent(pCanvas2);
		pFlow->AddComponent(pCanvas3);
		pCanvas1->SetDesiredSize(10, 10);
		pCanvas2->SetDesiredSize(20, 20);
		pCanvas3->SetDesiredSize(5, 16);
		pCanvas1->SetAlignment(CSH_Left, CSV_Top);
		pCanvas2->SetAlignment(CSH_Left, CSV_Top);
		pCanvas3->SetAlignment(CSH_Left, CSV_Top);

		AssertSize(3, pFlow->NumComponents());
		sVec = pCanvas1->GetActualSize();
		AssertInt32Vec2(0, 0, &sVec);

		pTestWindow->Show();

		sVec = pCanvas1->GetActualSize();
		AssertInt32Vec2(10, 10, &sVec);
		sVec = pCanvas2->GetActualSize();
		AssertInt32Vec2(20, 20, &sVec);
		sVec = pCanvas3->GetActualSize();
		AssertInt32Vec2(5, 16, &sVec);
		sVec = pCanvas1->GetPosition();
		AssertInt32Vec2(0, 0, &sVec);
		sVec = pCanvas2->GetPosition();
		AssertInt32Vec2(10, 0, &sVec);
		sVec = pCanvas3->GetPosition();
		AssertInt32Vec2(30, 0, &sVec);

		pCanvas1->GetContainerBounds(&cBounds);

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();

		ORoot()->Remove(pTestWindow);
	}

	AssertFlowContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerBasicOnStack(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "FlowContainerBasic";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		CWindow				cTestWindow;
		CChars				szExpectedDirectory;
		CCanvas				cCanvas1;
		CCanvas				cCanvas2;
		CCanvas				cCanvas3;
		CBorderCanvasDraw	cDraw1;
		CBorderCanvasDraw	cDraw2;
		CBorderCanvasDraw	cDraw3;
		CTickTestRefWindow	cTick;
		SDataTestRefWindow	cData;
		CFlowContainer		cFlow;
		CInt32Rect			cBounds;
		SInt32Vec2			sVec;

		cNativeFactory.Init(&gcMemoryAllocator, 96, 30, szDirectory);

		cTick.Init(&cData, 1);
		cTestWindow.Init("Reference Test Window", &cNativeFactory, &cTick, NULL, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);

		cDraw1.Init(Set32BitColour(1.0f, 0, 0));
		cCanvas1.Init(&cTestWindow, &cDraw1);
		cDraw2.Init(Set32BitColour(0, 1.0f, 0));
		cCanvas2.Init(&cTestWindow, &cDraw2);
		cDraw3.Init(Set32BitColour(0, 0, 1.0f));
		cCanvas3.Init(&cTestWindow, &cDraw3);

		cFlow.Init(&cTestWindow);
		cFlow.SetFlowStyle(CSD_Right, CSW_Continue, CSV_Top);
		cTestWindow.SetContainer(&cFlow);

		cFlow.AddComponent(&cCanvas1);
		cFlow.AddComponent(&cCanvas2);
		cFlow.AddComponent(&cCanvas3);
		cCanvas1.SetDesiredSize(10, 10);
		cCanvas2.SetDesiredSize(20, 20);
		cCanvas3.SetDesiredSize(5, 16);
		cCanvas1.SetAlignment(CSH_Left, CSV_Top);
		cCanvas2.SetAlignment(CSH_Left, CSV_Top);
		cCanvas3.SetAlignment(CSH_Left, CSV_Top);

		AssertSize(3, cFlow.NumComponents());
		sVec = cCanvas1.GetActualSize();
		AssertInt32Vec2(0, 0, &sVec);

		cTestWindow.Show();

		sVec = cCanvas1.GetActualSize();
		AssertInt32Vec2(10, 10, &sVec);
		sVec = cCanvas2.GetActualSize();
		AssertInt32Vec2(20, 20, &sVec);
		sVec = cCanvas3.GetActualSize();
		AssertInt32Vec2(5, 16, &sVec);
		sVec = cCanvas1.GetPosition();
		AssertInt32Vec2(0, 0, &sVec);
		sVec = cCanvas2.GetPosition();
		AssertInt32Vec2(10, 0, &sVec);
		sVec = cCanvas3.GetPosition();
		AssertInt32Vec2(30, 0, &sVec);

		cCanvas1.GetContainerBounds(&cBounds);

		cCanvas1.Kill();
		cCanvas2.Kill();
		cCanvas3.Kill();
		cFlow.Kill();
		cTestWindow.Kill();

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}

	AssertFlowContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerRightCenteredContinue(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	size					uiStartObjects;
	char					szDirectory[] = "Output" _FS_ "FlowContainerRightCenteredContinue";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	uiStartObjects = gcObjects.NumMemoryIndexes();
	{
		Ptr<CWindow>							pTestWindow;
		size									i;
		CChars									szExpectedDirectory;
		Ptr<CCanvas>							pCanvas;
		Ptr<CBorderCanvasDraw>					pDraw;
		CTickTestRefWindow						cTick;
		SDataTestRefWindow						cData;
		Ptr<CFlowContainer>						pFlow;
		CInt32Rect								cBounds;
		ARGB32									sColour;
		CRandom									cRandom;
		CPointer								pNull;
		CArrayBaseObjectPtrE32					apcHeapFroms;
		CArrayStackPointerE32					apcStackFroms;
		size									uiNumHeaps;
		CBaseObject*							pcWindowObject;
		CBaseObject*							pcCanvasObject1a;
		CBaseObject*							pcCanvasObject1b;
		CBaseObject*							pcFocusObject;
		CPointer*								pcWindowPointer;
		SStackPointer*							psStackPointer;

		cNativeFactory.Init(&gcMemoryAllocator, 96, 24, szDirectory);

		cTick.Init(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, &cTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		AssertSize(4, pTestWindow.NumHeapFroms());
		apcHeapFroms.Init();
		pTestWindow->GetHeapFroms(&apcHeapFroms);
		pcWindowObject = *apcHeapFroms.Get(0);
		pcCanvasObject1a = *apcHeapFroms.Get(1);
		pcCanvasObject1b = *apcHeapFroms.Get(2);
		pcFocusObject = *apcHeapFroms.Get(3);
		apcHeapFroms.Kill();

		AssertPointer(&pTestWindow, pcWindowObject);
		AssertPointer(&pTestWindow->GetCanvas(), pcCanvasObject1a);
		AssertPointer(&pTestWindow->GetCanvas(), pcCanvasObject1b);
		AssertPointer(&pTestWindow->GetFocus(), pcFocusObject);

		AssertSize(1, pTestWindow.NumStackFroms());
		apcStackFroms.Init();
		pTestWindow->GetStackFroms(&apcStackFroms);
		psStackPointer = apcStackFroms.Get(0);
		pcWindowPointer = psStackPointer->u.pcPointer;
		apcStackFroms.Kill();
		AssertPointer(pTestWindow.This(), pcWindowPointer);

		pFlow = OMalloc<CFlowContainer>(pTestWindow);

		pFlow->SetFlowStyle(CSD_Right, CSW_Continue, CSV_Center);
		pTestWindow->SetContainer(pFlow);
		AssertSize(uiStartObjects + 3, gcObjects.NumMemoryIndexes());

		cRandom.Init(6785);
		for (i = 0; i < 10; i++)
		{
			sColour = Set32BitColour(cRandom.Next(0, 255) / 255.0f, cRandom.Next(0, 255) / 255.0f, cRandom.Next(0, 255) / 255.0f);
			pDraw = OMalloc<CBorderCanvasDraw>(sColour);

			pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
			pFlow->AddComponent(pCanvas);
			pCanvas->SetAlignment(CSH_Left, CSV_Center);

			if (i % 2 == 0)
			{
				pCanvas->SetDesiredSize(5 + (int)i, 5 + (int)i);
			}
			else
			{
				pCanvas->SetDesiredSize(5 + (10 - (int)i), 5 + (10 - (int)i));
			}
			pCanvas = NULL;
			pDraw = NULL;

			AssertSize(uiStartObjects + 3 + (i + 1) * 2, gcObjects.NumMemoryIndexes());
			AssertSize(6 + i, pTestWindow.NumHeapFroms());
			AssertSize(1, pTestWindow.NumStackFroms());

			apcHeapFroms.Init();
			pTestWindow->GetHeapFroms(&apcHeapFroms);
			pcWindowObject = *apcHeapFroms.Get(0);
			pcCanvasObject1a = *apcHeapFroms.Get(1);
			pcCanvasObject1b = *apcHeapFroms.Get(2);
			pcFocusObject = *apcHeapFroms.Get(3);
			apcHeapFroms.Kill();

			AssertPointer(&pTestWindow, pcWindowObject);
			AssertPointer(&pTestWindow->GetCanvas(), pcCanvasObject1a);
			AssertPointer(&pTestWindow->GetCanvas(), pcCanvasObject1b);
			AssertPointer(&pTestWindow->GetFocus(), pcFocusObject);
		}

		AssertSize(10, pFlow->NumComponents());
		AssertSize(1, pFlow.NumStackFroms());
		apcStackFroms.Init();
		pFlow->GetStackFroms(&apcStackFroms);
		apcStackFroms.Kill();
		pFlow = NULL;

		pTestWindow->Show();
		AssertSize(uiStartObjects + 2 + 56, gcObjects.NumMemoryIndexes());  //+2 for GraphFreeObjects and its Set.  Each of the 11 native canvases holds a CImageCel over its image, and the window's image holds a blitter cache with a blitter for each of the 10 canvases drawn onto it.

		AssertSize(1, pTestWindow.NumStackFroms());
		apcStackFroms.Init();
		pTestWindow->GetStackFroms(&apcStackFroms);
		pcWindowPointer = apcStackFroms.Get(0)->u.pcPointer;
		apcStackFroms.Kill();
		AssertPointer(pTestWindow.This(), pcWindowPointer);

		AssertSize(15, pTestWindow.NumHeapFroms());
		apcHeapFroms.Init();
		pTestWindow->GetHeapFroms(&apcHeapFroms);
		uiNumHeaps = apcHeapFroms.NumElements();
		for (i = 0; i < uiNumHeaps; i++)
		{
		}
		apcHeapFroms.Kill();

		pTestWindow = NULL;
		AssertSize(uiStartObjects + 2, gcObjects.NumMemoryIndexes());  //GraphFreeObjects and its Set.

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}

	AssertFlowContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerRightTopContinue(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	size					uiStartObjects;
	char					szDirectory[] = "Output" _FS_ "FlowContainerRightTopContinue";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	uiStartObjects = gcObjects.NumMemoryIndexes();
	{
		Ptr<CWindow>			pTestWindow;
		size					i;
		CChars					szExpectedDirectory;
		Ptr<CCanvas>			pCanvas;
		Ptr<CBorderCanvasDraw>	pDraw;
		CTickTestRefWindow		cTick;
		SDataTestRefWindow		cData;
		Ptr<CFlowContainer>		pFlow;
		CInt32Rect				cBounds;
		ARGB32					sColour;
		CRandom					cRandom;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 96, 24, szDirectory);

		cTick.Init(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, &cTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		AssertSize(4, pTestWindow.NumHeapFroms());


		pFlow = OMalloc<CFlowContainer>(pTestWindow);

		pFlow->SetFlowStyle(CSD_Right, CSW_Continue, CSV_Top);
		pTestWindow->SetContainer(pFlow);

		cRandom.Init(2945);
		for (i = 0; i < 10; i++)
		{
			sColour = Set32BitColour(cRandom.Next(0, 255) / 255.0f, cRandom.Next(0, 255) / 255.0f, cRandom.Next(0, 255) / 255.0f);
			pDraw = OMalloc<CBorderCanvasDraw>(sColour);

			pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
			pFlow->AddComponent(pCanvas);
			pCanvas->SetAlignment(CSH_Left, CSV_Top);

			if (i % 2 == 0)
			{
				pCanvas->SetDesiredSize(5 + (int)i, 5 + (int)i);
			}
			else
			{
				pCanvas->SetDesiredSize(5 + (10 - (int)i), 5 + (10 - (int)i));
			}
			pCanvas = NULL;
			pDraw = NULL;

			AssertSize(uiStartObjects + 3 + (i + 1) * 2, gcObjects.NumMemoryIndexes());
			AssertSize(6 + i, pTestWindow.NumHeapFroms());
			AssertSize(1, pTestWindow.NumStackFroms());
		}

		pTestWindow->Show();


		AssertSize(uiStartObjects + 2 + 56, gcObjects.NumMemoryIndexes());  //+2 for GraphFreeObjects and its Set.  Each of the 11 native canvases holds a CImageCel over its image, and the window's image holds a blitter cache with a blitter for each of the 10 canvases drawn onto it.
		pTestWindow = NULL;
		AssertSize(uiStartObjects + 2 + 56, gcObjects.NumMemoryIndexes());  //+2 for GraphFreeObjects and its Set.  Each of the 11 native canvases holds a CImageCel over its image, and the window's image holds a blitter cache with a blitter for each of the 10 canvases drawn onto it.
		pFlow = NULL;
		AssertSize(uiStartObjects + 2, gcObjects.NumMemoryIndexes());  //GraphFreeObjects and its Set.

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}

	AssertFlowContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerRightBottomContinue(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	size					uiStartObjects;
	char					szDirectory[] = "Output" _FS_ "FlowContainerRightBottomContinue";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	uiStartObjects = gcObjects.NumMemoryIndexes();
	{
		Ptr<CWindow>			pTestWindow;
		size					i;
		CChars					szExpectedDirectory;
		Ptr<CCanvas>			pCanvas;
		Ptr<CBorderCanvasDraw>	pDraw;
		CTickTestRefWindow		cTick;
		SDataTestRefWindow		cData;
		Ptr<CFlowContainer>		pFlow;
		CInt32Rect				cBounds;
		ARGB32					sColour;
		CRandom					cRandom;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 96, 24, szDirectory);

		cTick.Init(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, &cTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		AssertSize(4, pTestWindow.NumHeapFroms());


		pFlow = OMalloc<CFlowContainer>(pTestWindow);

		pFlow->SetFlowStyle(CSD_Right, CSW_Continue, CSV_Bottom);
		pTestWindow->SetContainer(pFlow);

		cRandom.Init(3563);
		for (i = 0; i < 10; i++)
		{
			sColour = Set32BitColour(cRandom.Next(0, 255) / 255.0f, cRandom.Next(0, 255) / 255.0f, cRandom.Next(0, 255) / 255.0f);
			pDraw = OMalloc<CBorderCanvasDraw>(sColour);

			pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
			pFlow->AddComponent(pCanvas);
			pCanvas->SetAlignment(CSH_Left, CSV_Bottom);

			if (i % 2 == 0)
			{
				pCanvas->SetDesiredSize(5 + (int)i, 5 + (int)i);
			}
			else
			{
				pCanvas->SetDesiredSize(5 + (10 - (int)i), 5 + (10 - (int)i));
			}
			pCanvas = NULL;
			pDraw = NULL;

			AssertSize(uiStartObjects + 3 + (i + 1) * 2, gcObjects.NumMemoryIndexes());
			AssertSize(6 + i, pTestWindow.NumHeapFroms());
			AssertSize(1, pTestWindow.NumStackFroms());
		}

		pTestWindow->Show();


		AssertSize(uiStartObjects + 2 + 56, gcObjects.NumMemoryIndexes());  //+2 for GraphFreeObjects and its Set.  Each of the 11 native canvases holds a CImageCel over its image, and the window's image holds a blitter cache with a blitter for each of the 10 canvases drawn onto it.
		pTestWindow = NULL;
		AssertSize(uiStartObjects + 2 + 56, gcObjects.NumMemoryIndexes());  //+2 for GraphFreeObjects and its Set.  Each of the 11 native canvases holds a CImageCel over its image, and the window's image holds a blitter cache with a blitter for each of the 10 canvases drawn onto it.
		pFlow = NULL;
		AssertSize(uiStartObjects + 2, gcObjects.NumMemoryIndexes());  //GraphFreeObjects and its Set.

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}

	AssertFlowContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerRightTopWrap(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	size					uiStartObjects;
	char					szDirectory[] = "Output" _FS_ "FlowContainerRightTopWrap";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	uiStartObjects = gcObjects.NumMemoryIndexes();
	{
		Ptr<CWindow>			pTestWindow;
		size					i;
		CChars					szExpectedDirectory;
		Ptr<CCanvas>			pCanvas;
		Ptr<CBorderCanvasDraw>	pDraw;
		CTickTestRefWindow		cTick;
		SDataTestRefWindow		cData;
		Ptr<CFlowContainer>		pFlow;
		CInt32Rect				cBounds;
		ARGB32					sColour;
		CRandom					cRandom;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 32, 96, szDirectory);

		cTick.Init(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, &cTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		AssertSize(4, pTestWindow.NumHeapFroms());


		pFlow = OMalloc<CFlowContainer>(pTestWindow);

		pFlow->SetFlowStyle(CSD_Right, CSW_Wrap, CSV_Top);
		pTestWindow->SetContainer(pFlow);

		cRandom.Init(2945);
		for (i = 0; i < 10; i++)
		{
			sColour = Set32BitColour(cRandom.Next(0, 255) / 255.0f, cRandom.Next(0, 255) / 255.0f, cRandom.Next(0, 255) / 255.0f);
			pDraw = OMalloc<CBorderCanvasDraw>(sColour);

			pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
			pFlow->AddComponent(pCanvas);
			pCanvas->SetAlignment(CSH_Left, CSV_Top);

			if (i % 2 == 0)
			{
				pCanvas->SetDesiredSize(5 + (int)i, 5 + (int)i);
			}
			else
			{
				pCanvas->SetDesiredSize(5 + (10 - (int)i), 5 + (10 - (int)i));
			}
			pCanvas = NULL;
			pDraw = NULL;

			AssertSize(uiStartObjects + 3 + (i + 1) * 2, gcObjects.NumMemoryIndexes());
			AssertSize(6 + i, pTestWindow.NumHeapFroms());
			AssertSize(1, pTestWindow.NumStackFroms());
		}

		pTestWindow->Show();


		AssertSize(uiStartObjects + 2 + 56, gcObjects.NumMemoryIndexes());  //+2 for GraphFreeObjects and its Set.  Each of the 11 native canvases holds a CImageCel over its image, and the window's image holds a blitter cache with a blitter for each of the 10 canvases drawn onto it.
		pTestWindow = NULL;
		AssertSize(uiStartObjects + 2 + 56, gcObjects.NumMemoryIndexes());  //+2 for GraphFreeObjects and its Set.  Each of the 11 native canvases holds a CImageCel over its image, and the window's image holds a blitter cache with a blitter for each of the 10 canvases drawn onto it.
		pFlow = NULL;
		AssertSize(uiStartObjects + 2, gcObjects.NumMemoryIndexes());  //GraphFreeObjects and its Set.

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}

	AssertFlowContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainer(char* szDirectory, int32 iWidth, int32 iHeight, uint32 iRandomSeed, EContainerStyleDirection eD, EContainerStyleWrap eW, EContainerStyleVertical eV, EContainerStyleHorizontal eH)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	size					uiStartObjects;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	uiStartObjects = gcObjects.NumMemoryIndexes();
	{
		Ptr<CWindow>			pTestWindow;
		size					i;
		CChars					szExpectedDirectory;
		Ptr<CCanvas>			pCanvas;
		Ptr<CBorderCanvasDraw>	pDraw;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		Ptr<CFlowContainer>		pFlow;
		CInt32Rect				cBounds;
		ARGB32					sColour;
		CRandom					cRandom;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, iWidth, iHeight, szDirectory);

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFlow = OMalloc<CFlowContainer>(pTestWindow);
		if ((eV != CSV_Unknown) && (eH == CSH_Unknown))
		{
			pFlow->SetFlowStyle(eD, eW, eV);
		}
		if ((eV == CSV_Unknown) && (eH != CSH_Unknown))
		{
			pFlow->SetFlowStyle(eD, eW, eH);
		}
		pTestWindow->SetContainer(pFlow);

		cRandom.Init(iRandomSeed);
		for (i = 0; i < 10; i++)
		{
			sColour = Set32BitColour(cRandom.Next(0, 255) / 255.0f, cRandom.Next(0, 255) / 255.0f, cRandom.Next(0, 255) / 255.0f);
			pDraw = OMalloc<CBorderCanvasDraw>(sColour);

			pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
			pFlow->AddComponent(pCanvas);
			pCanvas->SetAlignment(eH, eV);  //The flow's alignment places the block of lines, the component's places it within its line.

			if (i % 2 == 0)
			{
				pCanvas->SetDesiredSize(5 + (int)i, 5 + (int)i);
			}
			else
			{
				pCanvas->SetDesiredSize(5 + (10 - (int)i), 5 + (10 - (int)i));
			}
			pCanvas = NULL;
			pDraw = NULL;
		}

		pTestWindow->Show();


		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pFlow = NULL;
		AssertSize(uiStartObjects + 2, gcObjects.NumMemoryIndexes());  //GraphFreeObjects and its Set.

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}

	AssertFlowContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainer(char* szDirectory, int32 iWidth, int32 iHeight, uint32 iRandomSeed, EContainerStyleDirection eD, EContainerStyleWrap eW, EContainerStyleVertical eA)
{
	TestFlowContainer(szDirectory, iWidth, iHeight, iRandomSeed, eD, eW, eA, CSH_Unknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainer(char* szDirectory, int32 iWidth, int32 iHeight, uint32 iRandomSeed, EContainerStyleDirection eD, EContainerStyleWrap eW, EContainerStyleHorizontal eA)
{
	TestFlowContainer(szDirectory, iWidth, iHeight, iRandomSeed, eD, eW, CSV_Unknown, eA);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerRightCenterWrap(void)
{
	TestFlowContainer("Output" _FS_ "FlowContainerRightCenterWrap", 32, 96, 2945, CSD_Right, CSW_Wrap, CSV_Center);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerRightBottomWrap(void)
{
	TestFlowContainer("Output" _FS_ "FlowContainerRightBottomWrap", 32, 96, 2945, CSD_Right, CSW_Wrap, CSV_Bottom);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowLongContainerRightBottomWrap(void)
{
	TestFlowContainer("Output" _FS_ "FlowLongContainerRightBottomWrap", 96, 14, 6356, CSD_Right, CSW_Wrap, CSV_Bottom);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDownCenterContinue(void)
{
	TestFlowContainer("Output" _FS_ "FlowContainerDownCenterContinue", 32, 96, 7548, CSD_Down, CSW_Continue, CSH_Center);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDownLeftContinue(void)
{
	TestFlowContainer("Output" _FS_ "FlowContainerDownLeftContinue", 32, 96, 7548, CSD_Down, CSW_Continue, CSH_Left);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDownRightContinue(void)
{
	TestFlowContainer("Output" _FS_ "FlowContainerDownRightContinue", 32, 96, 7548, CSD_Down, CSW_Continue, CSH_Right);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDownCenterWrap(void)
{
	TestFlowContainer("Output" _FS_ "FlowContainerDownCenterWrap", 96, 32, 8129, CSD_Down, CSW_Wrap, CSH_Center);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDownLeftWrap(void)
{
	TestFlowContainer("Output" _FS_ "FlowContainerDownLeftWrap", 96, 32, 8129, CSD_Down, CSW_Wrap, CSH_Left);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDownRightWrap(void)
{
	TestFlowContainer("Output" _FS_ "FlowContainerDownRightWrap", 96, 32, 8129, CSD_Down, CSW_Wrap, CSH_Right);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowLongContainerDownRightWrap(void)
{
	TestFlowContainer("Output" _FS_ "FlowLongContainerDownRightWrap", 14, 96, 2941, CSD_Down, CSW_Wrap, CSH_Right);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerPositions(char* szDirectory, int32 iWidth, int32 iHeight, EContainerStyleDirection eD, EContainerStyleWrap eW, EContainerStyleVertical eV, EContainerStyleHorizontal eH, size uiNumComponents, SInt32Vec2* asDesiredSizes, SInt32Vec2* asExpectedPositions, SInt32Vec2* asExpectedSizes, EContainerStyleHorizontal* aeHorizontal = NULL, EContainerStyleVertical* aeVertical = NULL, int* paiInsets = NULL)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	size					uiStartObjects;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	uiStartObjects = gcObjects.NumMemoryIndexes();
	{
		Ptr<CWindow>			pTestWindow;
		size					i;
		Ptr<CCanvas>			apCanvas[4];
		SInsets					sInsets;
		Ptr<CBorderCanvasDraw>	pDraw;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		Ptr<CFlowContainer>		pFlow;
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, iWidth, iHeight, szDirectory);

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFlow = OMalloc<CFlowContainer>(pTestWindow);
		if (eH == CSH_Unknown)
		{
			pFlow->SetFlowStyle(eD, eW, eV);
		}
		else
		{
			pFlow->SetFlowStyle(eD, eW, eH);
		}
		if (paiInsets)
		{
			sInsets.Init(paiInsets);
			pFlow->SetInsets(&sInsets);
		}
		pTestWindow->SetContainer(pFlow);

		for (i = 0; i < uiNumComponents; i++)
		{
			pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0));
			apCanvas[i] = OMalloc<CCanvas>(pTestWindow, pDraw);
			pFlow->AddComponent(apCanvas[i]);
			apCanvas[i]->SetDesiredSize(asDesiredSizes[i].x, asDesiredSizes[i].y);
			if (aeHorizontal != NULL)
			{
				apCanvas[i]->SetAlignment(aeHorizontal[i], aeVertical[i]);
			}
			else
			{
				apCanvas[i]->SetAlignment(eH, eV);
			}
		}
		pDraw = NULL;

		pTestWindow->Show();

		for (i = 0; i < uiNumComponents; i++)
		{
			sVec = apCanvas[i]->GetPosition();
			AssertInt32Vec2(asExpectedPositions[i].x, asExpectedPositions[i].y, &sVec);
			sVec = apCanvas[i]->GetActualSize();
			AssertInt32Vec2(asExpectedSizes[i].x, asExpectedSizes[i].y, &sVec);
			apCanvas[i] = NULL;
		}

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pFlow = NULL;
		AssertSize(uiStartObjects + 2, gcObjects.NumMemoryIndexes());  //GraphFreeObjects and its Set.
	}

	AssertFlowContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerLeftTopContinue(void)
{
	SInt32Vec2	asDesired[3] = { SInt32Vec2(10, 10), SInt32Vec2(20, 20), SInt32Vec2(5, 16) };
	SInt32Vec2	asPosition[3] = { SInt32Vec2(86, 0), SInt32Vec2(66, 0), SInt32Vec2(61, 0) };

	TestFlowContainerPositions("Output" _FS_ "FlowContainerLeftTopContinue", 96, 30, CSD_Left, CSW_Continue, CSV_Top, CSH_Unknown, 3, asDesired, asPosition, asDesired);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerLeftBottomWrap(void)
{
	SInt32Vec2	asDesired[3] = { SInt32Vec2(40, 10), SInt32Vec2(30, 20), SInt32Vec2(40, 5) };
	SInt32Vec2	asPosition[3] = { SInt32Vec2(56, 15), SInt32Vec2(26, 5), SInt32Vec2(56, 25) };

	TestFlowContainerPositions("Output" _FS_ "FlowContainerLeftBottomWrap", 96, 30, CSD_Left, CSW_Wrap, CSV_Bottom, CSH_Unknown, 3, asDesired, asPosition, asDesired);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerUpRightContinue(void)
{
	SInt32Vec2	asDesired[3] = { SInt32Vec2(10, 10), SInt32Vec2(20, 20), SInt32Vec2(5, 16) };
	SInt32Vec2	asPosition[3] = { SInt32Vec2(86, 50), SInt32Vec2(76, 30), SInt32Vec2(91, 14) };

	TestFlowContainerPositions("Output" _FS_ "FlowContainerUpRightContinue", 96, 60, CSD_Up, CSW_Continue, CSV_Unknown, CSH_Right, 3, asDesired, asPosition, asDesired);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerUpLeftWrap(void)
{
	SInt32Vec2	asDesired[3] = { SInt32Vec2(8, 10), SInt32Vec2(12, 15), SInt32Vec2(6, 10) };
	SInt32Vec2	asPosition[3] = { SInt32Vec2(0, 20), SInt32Vec2(0, 5), SInt32Vec2(12, 20) };

	TestFlowContainerPositions("Output" _FS_ "FlowContainerUpLeftWrap", 96, 30, CSD_Up, CSW_Wrap, CSV_Unknown, CSH_Left, 3, asDesired, asPosition, asDesired);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDownStretchContinue(void)
{
	SInt32Vec2	asDesired[2] = { SInt32Vec2(10, 10), SInt32Vec2(20, 20) };
	SInt32Vec2	asPosition[2] = { SInt32Vec2(0, 0), SInt32Vec2(0, 10) };
	SInt32Vec2	asSize[2] = { SInt32Vec2(96, 10), SInt32Vec2(96, 20) };

	TestFlowContainerPositions("Output" _FS_ "FlowContainerDownStretchContinue", 96, 30, CSD_Down, CSW_Continue, CSV_Unknown, CSH_Stretch, 2, asDesired, asPosition, asSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerRightComponentAlignmentContinue(void)
{
	//Each component places itself across the line by its own alignment.  Without wrapping the line is the whole area.
	SInt32Vec2					asDesired[4] = { SInt32Vec2(10, 10), SInt32Vec2(10, 10), SInt32Vec2(10, 10), SInt32Vec2(10, 10) };
	SInt32Vec2					asPosition[4] = { SInt32Vec2(0, 0), SInt32Vec2(10, 10), SInt32Vec2(20, 20), SInt32Vec2(30, 0) };
	SInt32Vec2					asSize[4] = { SInt32Vec2(10, 10), SInt32Vec2(10, 10), SInt32Vec2(10, 10), SInt32Vec2(10, 30) };
	EContainerStyleHorizontal	aeHorizontal[4] = { CSH_Left, CSH_Left, CSH_Left, CSH_Left };
	EContainerStyleVertical		aeVertical[4] = { CSV_Top, CSV_Center, CSV_Bottom, CSV_Stretch };

	TestFlowContainerPositions("Output" _FS_ "FlowContainerRightComponentAlignmentContinue", 96, 30, CSD_Right, CSW_Continue, CSV_Top, CSH_Unknown, 4, asDesired, asPosition, asSize, aeHorizontal, aeVertical);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerRightComponentAlignmentWrap(void)
{
	//The flow's bottom alignment puts the 26 high block of lines at the bottom of the 40 high area.  Within the first 20
	//thick line each component places itself by its own alignment.
	SInt32Vec2					asDesired[4] = { SInt32Vec2(10, 20), SInt32Vec2(10, 10), SInt32Vec2(10, 10), SInt32Vec2(20, 6) };
	SInt32Vec2					asPosition[4] = { SInt32Vec2(0, 14), SInt32Vec2(10, 14), SInt32Vec2(20, 24), SInt32Vec2(0, 34) };
	EContainerStyleHorizontal	aeHorizontal[4] = { CSH_Left, CSH_Left, CSH_Left, CSH_Left };
	EContainerStyleVertical		aeVertical[4] = { CSV_Top, CSV_Top, CSV_Bottom, CSV_Center };

	TestFlowContainerPositions("Output" _FS_ "FlowContainerRightComponentAlignmentWrap", 30, 40, CSD_Right, CSW_Wrap, CSV_Bottom, CSH_Unknown, 4, asDesired, asPosition, asDesired, aeHorizontal, aeVertical);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerInsets(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "FlowContainerInsets";
	int						aiInsets[4] = { 2, 3, 4, 5 };  //Top, Right, Bottom, Left.

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CCanvas>			apCanvas[3];
		Ptr<CBorderCanvasDraw>	pDraw;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		Ptr<CFlowContainer>		pFlow;
		SInt32Vec2				sVec;
		CInt32Rect				cBounds;
		CPointer				pNull;
		SInsets					sInsets;
		size					i;
		SInt32Vec2				asDesired[3] = { SInt32Vec2(10, 10), SInt32Vec2(20, 20), SInt32Vec2(5, 16) };

		cNativeFactory.Init(&gcMemoryAllocator, 96, 30, szDirectory);

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFlow = OMalloc<CFlowContainer>(pTestWindow);
		pFlow->SetFlowStyle(CSD_Right, CSW_Continue, CSV_Top);
		pTestWindow->SetContainer(pFlow);

		for (i = 0; i < 3; i++)
		{
			pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0));
			apCanvas[i] = OMalloc<CCanvas>(pTestWindow, pDraw);
			pFlow->AddComponent(apCanvas[i]);
			apCanvas[i]->SetDesiredSize(asDesired[i].x, asDesired[i].y);
			apCanvas[i]->SetAlignment(CSH_Left, CSV_Top);
		}
		pDraw = NULL;

		pFlow->SetRequiredSize();
		sVec = pFlow->GetRequiredSize();
		AssertInt32Vec2(35, 20, &sVec);

		//The insets add to the required size on both axes.
		sInsets.Init(aiInsets);
		pFlow->SetInsets(&sInsets);
		pFlow->SetRequiredSize();
		sVec = pFlow->GetRequiredSize();
		AssertInt32Vec2(43, 26, &sVec);

		//The components flow from the inside of the top left inset.  The container bounds are the area inside the insets
		//but the actual bounds are the whole area.
		pTestWindow->Show();

		sVec = apCanvas[0]->GetPosition();
		AssertInt32Vec2(5, 2, &sVec);
		sVec = apCanvas[1]->GetPosition();
		AssertInt32Vec2(15, 2, &sVec);
		sVec = apCanvas[2]->GetPosition();
		AssertInt32Vec2(35, 2, &sVec);
		for (i = 0; i < 3; i++)
		{
			sVec = apCanvas[i]->GetActualSize();
			AssertInt32Vec2(asDesired[i].x, asDesired[i].y, &sVec);
		}

		AssertTrue(pFlow->GetContainerBounds(&cBounds));
		AssertInt(5, cBounds.GetLeft());
		AssertInt(2, cBounds.GetTop());
		AssertInt(93, cBounds.GetRight());
		AssertInt(26, cBounds.GetBottom());
		sVec = pFlow->GetPosition();
		AssertInt32Vec2(0, 0, &sVec);
		sVec = pFlow->GetActualSize();
		AssertInt32Vec2(96, 30, &sVec);

		//Flowing down the main axis is the height.
		pFlow->SetFlowStyle(CSD_Down, CSW_Continue, CSH_Left);
		pFlow->SetRequiredSize();
		sVec = pFlow->GetRequiredSize();
		AssertInt32Vec2(28, 52, &sVec);

		for (i = 0; i < 3; i++)
		{
			apCanvas[i] = NULL;
		}
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pFlow = NULL;
	}

	AssertFlowContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerRightTopWrapInsets(void)
{
	//The two components would fit on one line of the 50 wide area but not on the 39 wide line inside the insets.
	int			aiInsets[4] = { 2, 6, 3, 5 };  //Top, Right, Bottom, Left.
	SInt32Vec2	asDesired[2] = { SInt32Vec2(20, 10), SInt32Vec2(20, 10) };
	SInt32Vec2	asPosition[2] = { SInt32Vec2(5, 2), SInt32Vec2(5, 12) };

	TestFlowContainerPositions("Output" _FS_ "FlowContainerRightTopWrapInsets", 50, 30, CSD_Right, CSW_Wrap, CSV_Top, CSH_Unknown, 2, asDesired, asPosition, asDesired, NULL, NULL, aiInsets);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerLeftBottomWrapInsets(void)
{
	//Like TestFlowContainerLeftBottomWrap but the lines are 88 long, right aligned against the right inset, and the 25
	//high block of lines sits on the bottom inset of the 28 high area inside the insets.
	int			aiInsets[4] = { 1, 3, 1, 5 };  //Top, Right, Bottom, Left.
	SInt32Vec2	asDesired[3] = { SInt32Vec2(40, 10), SInt32Vec2(30, 20), SInt32Vec2(40, 5) };
	SInt32Vec2	asPosition[3] = { SInt32Vec2(53, 14), SInt32Vec2(23, 4), SInt32Vec2(53, 24) };

	TestFlowContainerPositions("Output" _FS_ "FlowContainerLeftBottomWrapInsets", 96, 30, CSD_Left, CSW_Wrap, CSV_Bottom, CSH_Unknown, 3, asDesired, asPosition, asDesired, NULL, NULL, aiInsets);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerNested(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	size					uiStartObjects;
	char					szDirectory[] = "Output" _FS_ "FlowContainerNested";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	uiStartObjects = gcObjects.NumMemoryIndexes();
	{
		Ptr<CWindow>			pTestWindow;
		size					i;
		Ptr<CCanvas>			pCanvas;
		Ptr<CCanvas>			apInner[3];
		Ptr<CCanvas>			apRow[2];
		Ptr<CCanvas>			pHolder;
		Ptr<CBorderCanvasDraw>	pDraw;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		Ptr<CFlowContainer>		pOuterFlow;
		Ptr<CFlowContainer>		pInnerFlow;
		Ptr<CFlowContainer>		pRowFlow;
		SInt32Vec2				sVec;
		CInt32Rect				cBounds;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 96, 30, szDirectory);

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pOuterFlow = OMalloc<CFlowContainer>(pTestWindow);
		pOuterFlow->SetFlowStyle(CSD_Right, CSW_Continue, CSV_Top);
		pTestWindow->SetContainer(pOuterFlow);

		pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0));
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pCanvas->SetDesiredSize(10, 10);
		pCanvas->SetAlignment(CSH_Left, CSV_Top);
		pOuterFlow->AddComponent(pCanvas);

		//A wrapping flow inside a canvas that is not at the origin.
		pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(0, 1.0f, 0));
		pHolder = OMalloc<CCanvas>(pTestWindow, pDraw);
		pHolder->SetDesiredSize(40, 20);
		pHolder->SetAlignment(CSH_Left, CSV_Top);
		pOuterFlow->AddComponent(pHolder);

		pInnerFlow = OMalloc<CFlowContainer>(pTestWindow);
		pInnerFlow->SetFlowStyle(CSD_Right, CSW_Wrap, CSV_Top);
		pHolder->SetContainer(pInnerFlow);
		for (i = 0; i < 3; i++)
		{
			pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(0, 0, 1.0f));
			apInner[i] = OMalloc<CCanvas>(pTestWindow, pDraw);
			apInner[i]->SetDesiredSize(15, 5);
			pInnerFlow->AddComponent(apInner[i]);
		}

		//A flow added directly to a flow is laid out at its desired size.
		pRowFlow = OMalloc<CFlowContainer>(pTestWindow);
		pRowFlow->SetDesiredSize(7, 14);
		pRowFlow->SetAlignment(CSH_Left, CSV_Top);
		pRowFlow->SetFlowStyle(CSD_Down, CSW_Continue, CSH_Left);
		pOuterFlow->AddComponent(pRowFlow);
		for (i = 0; i < 2; i++)
		{
			pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 1.0f, 0));
			apRow[i] = OMalloc<CCanvas>(pTestWindow, pDraw);
			apRow[i]->SetDesiredSize(6 + (int)i, 7);
			apRow[i]->SetAlignment(CSH_Left, CSV_Top);
			pRowFlow->AddComponent(apRow[i]);
		}
		pCanvas = NULL;
		pDraw = NULL;

		pRowFlow->SetRequiredSize();
		sVec = pRowFlow->GetRequiredSize();
		AssertInt32Vec2(7, 14, &sVec);
		pOuterFlow->SetRequiredSize();
		sVec = pOuterFlow->GetRequiredSize();
		AssertInt32Vec2(57, 20, &sVec);
		sVec = pOuterFlow->GetDesiredSize();
		AssertInt32Vec2(-1, -1, &sVec);

		pTestWindow->Show();

		sVec = pHolder->GetPosition();
		AssertInt32Vec2(10, 0, &sVec);
		sVec = apInner[0]->GetPosition();
		AssertInt32Vec2(10, 0, &sVec);
		sVec = apInner[1]->GetPosition();
		AssertInt32Vec2(25, 0, &sVec);
		sVec = apInner[2]->GetPosition();
		AssertInt32Vec2(10, 5, &sVec);

		pRowFlow->GetContainerBounds(&cBounds);
		AssertInt32Vec2(50, 0, &cBounds.msTopLeft);
		AssertInt32Vec2(57, 14, &cBounds.msBottomRight);
		sVec = apRow[0]->GetPosition();
		AssertInt32Vec2(50, 0, &sVec);
		sVec = apRow[1]->GetPosition();
		AssertInt32Vec2(50, 7, &sVec);

		for (i = 0; i < 3; i++)
		{
			apInner[i] = NULL;
		}
		for (i = 0; i < 2; i++)
		{
			apRow[i] = NULL;
		}

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pOuterFlow = NULL;
		pInnerFlow = NULL;
		pRowFlow = NULL;
		pHolder = NULL;
		AssertSize(uiStartObjects + 2, gcObjects.NumMemoryIndexes());  //GraphFreeObjects and its Set.
	}

	AssertFlowContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDarkForest(char* szDirectory, int32 iWidth, int32 iHeight, EContainerStyleDirection eD, EContainerStyleWrap eW, EContainerStyleVertical eV, EContainerStyleHorizontal eH, size uiNumComponents, SDarkForestComponent* asComponents)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		size					i;
		CChars					szExpectedDirectory;
		Ptr<CCanvas>			apCanvas[16];
		Ptr<CCanvasDraw>		pDraw;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		Ptr<CFlowContainer>		pFlow;
		CColourFormatHelper		cHelper;
		SDarkForestComponent*	psComponent;
		SInt32Vec2				sDesired;
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, iWidth, iHeight, szDirectory);

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTestWindow->GetColourFormat(&cHelper);

		pFlow = OMalloc<CFlowContainer>(pTestWindow);
		if ((eV != CSV_Unknown) && (eH == CSH_Unknown))
		{
			pFlow->SetFlowStyle(eD, eW, eV);
		}
		if ((eV == CSV_Unknown) && (eH != CSH_Unknown))
		{
			pFlow->SetFlowStyle(eD, eW, eH);
		}
		AssertTrue(uiNumComponents <= 16);
		pTestWindow->SetContainer(pFlow);

		for (i = 0; i < uiNumComponents; i++)
		{
			psComponent = &asComponents[i];
			pDraw = CreateDarkForestDraw(psComponent, &cHelper);
			apCanvas[i] = OMalloc<CCanvas>(pTestWindow, pDraw);
			apCanvas[i]->SetDesiredSize(psComponent->sDesiredSize.x, psComponent->sDesiredSize.y);
			pFlow->AddComponent(apCanvas[i]);
			if ((psComponent->eHorizontal != CSH_Unknown) || (psComponent->eVertical != CSV_Unknown))
			{
				apCanvas[i]->SetAlignment(psComponent->eHorizontal, psComponent->eVertical);
			}
			else
			{
				apCanvas[i]->SetAlignment(eH, eV);
			}
			pDraw = NULL;
		}

		pTestWindow->Show();

		//Every canvas must have been laid out at the size it asked for, image canvases at the size of their cel unless overridden.
		for (i = 0; i < uiNumComponents; i++)
		{
			sDesired = apCanvas[i]->GetDesiredSize();
			AssertTrue((sDesired.x > 0) && (sDesired.y > 0));
			sVec = apCanvas[i]->GetActualSize();
			AssertInt32Vec2(sDesired.x, sDesired.y, &sVec);
			apCanvas[i] = NULL;
		}

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pFlow = NULL;

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}

	AssertFlowContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDarkForestImagesRightTopWrap(void)
{
	SDarkForestComponent	asComponents[9] = {
		{ "Pine.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Owl.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Moon.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Fern.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Mushroom.png",	SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Wolf.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Bat.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Fireflies.png",	SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Stump.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown } };

	TestFlowContainerDarkForest("Output" _FS_ "FlowContainerDarkForestImagesRightTopWrap", 40, 48, CSD_Right, CSW_Wrap, CSV_Top, CSH_Unknown, 9, asComponents);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDarkForestImagesDownCenterWrap(void)
{
	SDarkForestComponent	asComponents[9] = {
		{ "Moon.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Bat.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Pine.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Wolf.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Fireflies.png",	SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Owl.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Fern.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Mushroom.png",	SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Stump.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown } };

	TestFlowContainerDarkForest("Output" _FS_ "FlowContainerDarkForestImagesDownCenterWrap", 48, 32, CSD_Down, CSW_Wrap, CSV_Unknown, CSH_Center, 9, asComponents);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDarkForestMixedRightBottomWrap(void)
{
	SDarkForestComponent	asComponents[10] = {
		{ "Owl.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ NULL,				SInt32Vec2(6, 6),	Set32BitColour((uint8)60, 110, 60), CSH_Unknown, CSV_Unknown },
		{ "Pine.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ NULL,				SInt32Vec2(12, 5),	Set32BitColour((uint8)150, 30, 30), CSH_Unknown, CSV_Unknown },
		{ "Mushroom.png",	SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ NULL,				SInt32Vec2(4, 10),	Set32BitColour((uint8)236, 200, 70), CSH_Unknown, CSV_Unknown },
		{ "Wolf.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ NULL,				SInt32Vec2(8, 8),	Set32BitColour((uint8)78, 60, 96), CSH_Unknown, CSV_Unknown },
		{ "Fireflies.png",	SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Stump.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown } };

	TestFlowContainerDarkForest("Output" _FS_ "FlowContainerDarkForestMixedRightBottomWrap", 48, 36, CSD_Right, CSW_Wrap, CSV_Bottom, CSH_Unknown, 10, asComponents);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDarkForestMixedLeftCenterContinue(void)
{
	//The moon canvas is given a desired size larger than its cel so the cel sits in its top left with black beyond.
	SDarkForestComponent	asComponents[6] = {
		{ "Bat.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ NULL,				SInt32Vec2(5, 14),	Set32BitColour((uint8)110, 78, 48), CSH_Unknown, CSV_Unknown },
		{ "Moon.png",		SInt32Vec2(14, 14), 0, CSH_Unknown, CSV_Unknown },
		{ NULL,				SInt32Vec2(10, 4),	Set32BitColour((uint8)54, 104, 58), CSH_Unknown, CSV_Unknown },
		{ "Fern.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Owl.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown } };

	TestFlowContainerDarkForest("Output" _FS_ "FlowContainerDarkForestMixedLeftCenterContinue", 72, 16, CSD_Left, CSW_Continue, CSV_Center, CSH_Unknown, 6, asComponents);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainerDarkForestImagesRightTopContinuePastEdge(void)
{
	//Without wrapping the line carries on past the right edge of the 20 wide window.  The wolf fits, the pine at x 12
	//loses its last pixel and the moon at x 21 is entirely outside, so all three must be clipped to the window.
	SDarkForestComponent	asComponents[3] = {
		{ "Wolf.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Pine.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown },
		{ "Moon.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown } };

	TestFlowContainerDarkForest("Output" _FS_ "FlowContainerDarkForestImagesRightTopContinuePastEdge", 20, 16, CSD_Right, CSW_Continue, CSV_Top, CSH_Unknown, 3, asComponents);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFlowContainer(void)
{
	BeginTests();
	ObjectsInit();

	TestFlowContainerBasicOnHeap();
	TestFlowContainerBasicOnStack();
	TestFlowContainerRightCenteredContinue();
	TestFlowContainerRightTopContinue();
	TestFlowContainerRightBottomContinue();
	TestFlowContainerRightTopWrap();
	TestFlowContainerRightCenterWrap();
	TestFlowContainerRightBottomWrap();
	TestFlowLongContainerRightBottomWrap();
	TestFlowContainerDownCenterContinue();
	TestFlowContainerDownLeftContinue();
	TestFlowContainerDownRightContinue();
	TestFlowContainerDownCenterWrap();
	TestFlowContainerDownLeftWrap();
	TestFlowContainerDownRightWrap();
	TestFlowLongContainerDownRightWrap();
	TestFlowContainerLeftTopContinue();
	TestFlowContainerLeftBottomWrap();
	TestFlowContainerUpRightContinue();
	TestFlowContainerUpLeftWrap();
	TestFlowContainerDownStretchContinue();
	TestFlowContainerRightComponentAlignmentContinue();
	TestFlowContainerRightComponentAlignmentWrap();
	TestFlowContainerInsets();
	TestFlowContainerRightTopWrapInsets();
	TestFlowContainerLeftBottomWrapInsets();
	TestFlowContainerNested();
	TestFlowContainerDarkForestImagesRightTopWrap();
	TestFlowContainerDarkForestImagesDownCenterWrap();
	TestFlowContainerDarkForestMixedRightBottomWrap();
	TestFlowContainerDarkForestMixedLeftCenterContinue();
	TestFlowContainerDarkForestImagesRightTopContinuePastEdge();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

