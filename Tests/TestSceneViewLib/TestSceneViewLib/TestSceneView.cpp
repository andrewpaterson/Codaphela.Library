#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Instance.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/MeshBake.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshUnwrap.h"
#include "SupportLib/Scene.h"
#include "SupportLib/SolidChecker.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/Window.h"
#include "WindowLib/WindowTick.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "SceneViewLib/SceneView.h"
#include "TestLib/Assert.h"


#define TEST_SCENE_VIEW_WIDTH	160
#define TEST_SCENE_VIEW_HEIGHT	120


//Stops the window after a few ticks, by when it's been drawn.
class CTestSceneViewTick : public CWindowTick
{
CONSTRUCTABLE(CTestSceneViewTick);
DESTRUCTABLE(CTestSceneViewTick);
protected:
	int32	miTicks;

public:
	void Init(void)
	{
		PreInit();
		CWindowTick::Init();
		miTicks = 0;
		PostInit();
	}

	void Class(void) override
	{
		CWindowTick::Class();
		U_Int32(miTicks);
	}

	void Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) override
	{
		miTicks++;
		if (miTicks == 3)
		{
			pWindow->Stop();
		}
	}
};


//Counts the view's clicks.
struct STestSceneViewClicks
{
	int32		iClicks;
	SInt32Vec2	sWhere;

	void Clicked(CSceneView* pcView)
	{
		iClicks++;
		sWhere = pcView->GetClick();
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A scene of one box, two units a side about the point 3, 0, 0, with a
// checker baked onto it (a mesh with no material isn't drawn), and no
// camera.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CScene> TestSceneViewBox(void)
{
	Ptr<CScene>			pScene;
	Ptr<CMeshEditor>	pEditor;
	Ptr<CInstance>		pInstance;
	Ptr<CInstanceMesh>	pInstanceMesh;
	Ptr<CConnection>	pConnection;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;
	SFloat32Vec3		sZ;
	SFloat32Vec3		sStart;

	sX.Init(2, 0, 0);
	sY.Init(0, 2, 0);
	sZ.Init(0, 0, 2);
	sStart.Init(2, -1, -1);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshBoxEditor>(&sX, &sY, &sZ, &sStart, 1, 1, 1);
	pEditor->AddModifier<CMeshUnwrap>(8.0f);
	pEditor->AddModifier<CMeshBake>(OMalloc<CSolidChecker>(0.5f, Set32BitColour((uint8)230, (uint8)220, (uint8)200), Set32BitColour((uint8)60, (uint8)90, (uint8)150)));

	pConnection = OMalloc<CConnection>();
	pConnection->msWorldMatrix.Identity();
	pConnection->msLocalMatrix.Identity();
	pInstanceMesh = OMalloc<CInstanceMesh>(pEditor);
	pInstanceMesh->mapConnections.Add(pConnection);
	pInstance = OMalloc<CInstance>();
	pInstance->mapConnections.Add(pConnection);
	pInstance->mapMeshes.Add(pInstanceMesh);

	pScene = OMalloc<CScene>();
	pScene->mapInstances.Add(pInstance);
	return pScene;
}


//////////////////////////////////////////////////////////////////////////
//
// The B, G, R of the image's pixel, which the view drew.
//
//////////////////////////////////////////////////////////////////////////
static uint32 TestSceneViewPixel(Ptr<CImage> pImage, int32 x, int32 y)
{
	uint32	uiPixel;

	memcpy(&uiPixel, (uint8*)pImage->GetData() + ((size)y * pImage->GetWidth() + (size)x) * 4, 4);
	return uiPixel & 0x00FFFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
// An event at x, y.
//
//////////////////////////////////////////////////////////////////////////
static SComponentInputEvent TestSceneViewEvent(int32 x, int32 y, int32 iDown)
{
	SComponentInputEvent	sEvent;

	memset(&sEvent, 0, sizeof(sEvent));
	sEvent.x = x;
	sEvent.y = y;
	sEvent.iDown = iDown;
	return sEvent;
}


//////////////////////////////////////////////////////////////////////////
//
// A view of a box in a window with no screen, drawn by the rasteriser:
// the box is framed in the middle with the background round it, the
// middle of the view is the box's middle, the wheel goes closer, Home
// goes back, and a press and release where it was pressed is a click
// when something is told of clicks.
//
//////////////////////////////////////////////////////////////////////////
void TestSceneViewDrawnAndDriven(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "SceneView";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>			pWindow;
		Ptr<CGridContainer>		pGrid;
		Ptr<CSceneView>			pView;
		Ptr<CSceneCanvasDraw>	pDraw;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CImage>				pImage;
		CPointer				pNull;
		STestSceneViewClicks	sClicks;
		SComponentInputEvent	sEvent;
		SFloat32Vec3			sPoint;
		SFloat32Vec3			sCorner;
		SFloat32Vec3			sCloser;
		uint32					uiBackground;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, TEST_SCENE_VIEW_WIDTH, TEST_SCENE_VIEW_HEIGHT, szDirectory);
		pWindow = OMalloc<CWindow>("Scene View", &pNativeFactory, OMalloc<CTestSceneViewTick>(), pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);

		pTheme = OMalloc<CWindowTheme>(Ptr<CFont>());
		pTheme->SetColour(TC_Editor, Set32BitColour((uint8)40, (uint8)44, (uint8)52));
		pView = OMalloc<CSceneView>(pWindow, pTheme, (CLabel*)NULL, SVB_LeftTurns);
		pDraw = pView->GetDraw();
		AssertTrue(pDraw->GetImage().IsNull());
		AssertFalse(pDraw->ScreenToPlane(0, 0, &sPoint));  //Not drawn yet: it has no size.

		pDraw->UseVulkan(false);
		pDraw->SetScene(TestSceneViewBox(), "Box");
		pGrid = OMalloc<CGridContainer>(pWindow);
		pGrid->SetAlignment(CSH_Stretch, CSV_Stretch);
		pGrid->AddComponent(pView, 0, 0, 1, 1, 1, 1);
		pWindow->SetContainer(pGrid);
		pWindow->Show();

		pImage = pDraw->GetImage();
		AssertTrue(pImage.IsNotNull());
		AssertInt(TEST_SCENE_VIEW_WIDTH, (int32)pImage->GetWidth());
		AssertInt(TEST_SCENE_VIEW_HEIGHT, (int32)pImage->GetHeight());
		uiBackground = TestSceneViewPixel(pImage, 0, 0);
		AssertInt(uiBackground, TestSceneViewPixel(pImage, TEST_SCENE_VIEW_WIDTH - 1, TEST_SCENE_VIEW_HEIGHT - 1));
		AssertTrue(TestSceneViewPixel(pImage, TEST_SCENE_VIEW_WIDTH / 2, TEST_SCENE_VIEW_HEIGHT / 2) != uiBackground);

		AssertTrue(pDraw->ScreenToPlane(TEST_SCENE_VIEW_WIDTH / 2, TEST_SCENE_VIEW_HEIGHT / 2, &sPoint));
		AssertFloat3(SFloat32Vec3(3, 0, 0), &sPoint, 1);

		//Closer, the corner of the view is nearer the middle; Home puts it back.
		AssertTrue(pDraw->ScreenToPlane(0, 0, &sCorner));
		sEvent = TestSceneViewEvent(80, 60, -1);
		pView->WheelAction(&sEvent);
		AssertTrue(pDraw->ScreenToPlane(0, 0, &sCloser));
		AssertFloat((3.0f - sCorner.x) / SCENE_VIEW_ZOOM_FACTOR, 3.0f - sCloser.x, 3);
		pView->HomeAction(&sEvent);
		AssertTrue(pDraw->ScreenToPlane(0, 0, &sCloser));
		AssertFloat3(sCorner, &sCloser, 4);

		//Nothing is told of clicks, so a press isn't one.
		sClicks.iClicks = 0;
		sClicks.sWhere.Init(-1, -1);
		sEvent = TestSceneViewEvent(10, 20, 0);
		pView->PressAction(&sEvent);
		pView->ReleaseAction(&sEvent);
		AssertInt(0, sClicks.iClicks);

		pView->GetClickEvent()->Add(&sClicks, &STestSceneViewClicks::Clicked);
		pView->PressAction(&sEvent);
		pView->ReleaseAction(&sEvent);
		AssertInt(1, sClicks.iClicks);
		AssertInt(10, sClicks.sWhere.x);
		AssertInt(20, sClicks.sWhere.y);

		//Moved a little it's still a click; moved further it's a drag.
		pView->PressAction(&sEvent);
		sEvent = TestSceneViewEvent(10 + SCENE_VIEW_CLICK_PIXELS, 20, 0);
		pView->MovementAction(&sEvent);
		pView->ReleaseAction(&sEvent);
		AssertInt(2, sClicks.iClicks);

		sEvent = TestSceneViewEvent(30, 40, 0);
		pView->PressAction(&sEvent);
		sEvent = TestSceneViewEvent(31 + SCENE_VIEW_CLICK_PIXELS, 40, 0);
		pView->MovementAction(&sEvent);
		pView->ReleaseAction(&sEvent);
		AssertInt(2, sClicks.iClicks);
		pView->GetClickEvent()->Remove(&sClicks);

		pImage = NULL;
		pDraw = NULL;
		pView = NULL;
		pGrid = NULL;
		pTheme = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSceneView(void)
{
	BeginTests();

	TestSceneViewDrawnAndDriven();

	TestStatistics();
}

