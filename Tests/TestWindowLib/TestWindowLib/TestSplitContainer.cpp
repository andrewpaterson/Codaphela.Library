#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/SplitContainer.h"
#include "WindowLib/JewelTheme.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);


//Counts the times a split container's share changed.
class CTestSplitListener
{
public:
	int		miChanges;

	void ShareChanged(CSplitContainer* pcSplit)
	{
		miChanges++;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A split 300 wide: the first has its share of what the sash leaves and
// the second the rest; the sash dragged moves them, neither getting less
// than the minimum, and the share follows; the second hidden, the first
// has it all.  One above the other is the same down the window.
//
//////////////////////////////////////////////////////////////////////////
void TestSplitContainerShares(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pWindow;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CFillContainer>			pRoot;
		Ptr<CSplitContainer>		pSplit;
		Ptr<CFillContainer>			pFirst;
		Ptr<CFillContainer>			pSecond;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		CTestSplitListener			cListener;
		int32						iSash;
		int32						iRoom;

		cListener.miChanges = 0;
		cNativeFactory.Init(&gcMemoryAllocator, 300, 100, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pWindow = OMalloc<CWindow>("Split Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow));
		iSash = pTheme->GetMetric(TM_DockSashSize);
		iRoom = 300 - iSash;

		pRoot = OMalloc<CFillContainer>(pWindow);
		pWindow->SetContainer(pRoot);

		pSplit = OMalloc<CSplitContainer>(pWindow, pTheme, true);
		pSplit->SetAlignment(CSH_Stretch, CSV_Stretch);
		pSplit->GetShareEvent()->Add(&cListener, &CTestSplitListener::ShareChanged);
		pFirst = OMalloc<CFillContainer>(pWindow);
		pSecond = OMalloc<CFillContainer>(pWindow);
		pSplit->SetComponents(pFirst, pSecond);
		AssertFalse(pSplit->AddComponent(OMalloc<CFillContainer>(pWindow)));
		AssertTrue(pSplit->GetFirst() == pFirst);
		AssertTrue(pSplit->GetSecond() == pSecond);
		AssertTrue(pSplit->IsHorizontal());
		pRoot->AddComponent(pSplit);

		//Half each to start with, the sash between them, each as tall as the window.
		pWindow->Layout();
		AssertInt(SPLIT_CONTAINER_WHOLE / 2, pSplit->GetShare());
		AssertInt(iRoom / 2, pSplit->GetFirstSize());
		AssertInt(0, pFirst->GetPosition().x);
		AssertInt(iRoom / 2, pFirst->GetActualSize().x);
		AssertInt(100, pFirst->GetActualSize().y);
		AssertInt(iRoom / 2, pSplit->GetSash()->GetPosition().x);
		AssertInt(iSash, pSplit->GetSash()->GetActualSize().x);
		AssertInt(iRoom / 2 + iSash, pSecond->GetPosition().x);
		AssertInt(300 - iRoom / 2 - iSash, pSecond->GetActualSize().x);
		AssertInt(100, pSecond->GetActualSize().y);

		//A share given: the first has that much of the room, however wide the window.
		pSplit->SetShare(250);
		pWindow->Layout();
		AssertInt(iRoom / 4, pSplit->GetFirstSize());
		AssertInt(iRoom / 4 + iSash, pSecond->GetPosition().x);
		pSplit->SetShare(-5);
		AssertInt(0, pSplit->GetShare());
		pSplit->SetShare(5000);
		AssertInt(SPLIT_CONTAINER_WHOLE, pSplit->GetShare());
		pSplit->SetShare(250);
		pWindow->Layout();
		AssertInt(0, cListener.miChanges);

		//The sash dragged right: the first is that much wider, and the share is what it has.
		pSplit->SashMoved(20);
		pWindow->Layout();
		AssertInt(iRoom / 4 + 20, pSplit->GetFirstSize());
		AssertInt(iRoom / 4 + 20, pFirst->GetActualSize().x);
		AssertInt(((iRoom / 4 + 20) * SPLIT_CONTAINER_WHOLE + iRoom / 2) / iRoom, pSplit->GetShare());
		AssertInt(1, cListener.miChanges);

		//Neither side is dragged smaller than the minimum; a drag that moves nothing isn't a change.
		pSplit->SashMoved(-1000);
		pWindow->Layout();
		AssertInt(SPLIT_CONTAINER_MINIMUM, pSplit->GetFirstSize());
		AssertInt(2, cListener.miChanges);
		pSplit->SashMoved(-10);
		AssertInt(2, cListener.miChanges);
		pSplit->SashMoved(1000);
		pWindow->Layout();
		AssertInt(iRoom - SPLIT_CONTAINER_MINIMUM, pSplit->GetFirstSize());
		AssertInt(SPLIT_CONTAINER_MINIMUM, pSecond->GetActualSize().x);
		AssertInt(3, cListener.miChanges);

		//The second hidden: the first has all the room, the sash and the second none, and they aren't drawn.
		AssertTrue(pSplit->IsSecondShown());
		AssertTrue(pSplit->IsChildDrawn(pSecond));
		pSplit->SetSecondShown(false);
		pWindow->Layout();
		AssertFalse(pSplit->IsSecondShown());
		AssertInt(300, pFirst->GetActualSize().x);
		AssertInt(0, pSecond->GetActualSize().x);
		AssertInt(0, pSplit->GetSash()->GetActualSize().x);
		AssertTrue(pSplit->IsChildDrawn(pFirst));
		AssertFalse(pSplit->IsChildDrawn(pSecond));
		AssertFalse(pSplit->IsChildDrawn(pSplit->GetSash()));
		pSplit->SashMoved(-50);
		AssertInt(3, cListener.miChanges);

		//Shown again it has the share it had.
		pSplit->SetSecondShown(true);
		pWindow->Layout();
		AssertInt(iRoom - SPLIT_CONTAINER_MINIMUM, pSplit->GetFirstSize());

		pSplit->GetShareEvent()->Remove(&cListener);
		pRoot->RemoveComponent(pSplit);
		pSplit = NULL;
		pFirst = NULL;
		pSecond = NULL;

		//One above the other.
		iRoom = 100 - iSash;
		pSplit = OMalloc<CSplitContainer>(pWindow, pTheme, false);
		pFirst = OMalloc<CFillContainer>(pWindow);
		pSecond = OMalloc<CFillContainer>(pWindow);
		pSplit->SetAlignment(CSH_Stretch, CSV_Stretch);
		pSplit->SetComponents(pFirst, pSecond);
		pRoot->AddComponent(pSplit);
		pSplit->SetShare(500);
		pWindow->Layout();
		AssertFalse(pSplit->IsHorizontal());
		AssertInt(iRoom / 2, pFirst->GetActualSize().y);
		AssertInt(300, pFirst->GetActualSize().x);
		AssertInt(iRoom / 2, pSplit->GetSash()->GetPosition().y);
		AssertInt(iSash, pSplit->GetSash()->GetActualSize().y);
		AssertInt(iRoom / 2 + iSash, pSecond->GetPosition().y);
		AssertInt(300, pSecond->GetActualSize().x);

		pSplit = NULL;
		pFirst = NULL;
		pSecond = NULL;
		pRoot = NULL;
		pTheme = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSplitContainer(void)
{
	BeginTests();
	ObjectsInit();

	TestSplitContainerShares();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

