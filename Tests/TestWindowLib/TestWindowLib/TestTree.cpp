#include "BaseLib/GlobalMemory.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/TreeContainer.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CFont> ReadLabelFont(void);


//////////////////////////////////////////////////////////////////////////
//
// Logs the selection changing and nodes being activated.
//
//////////////////////////////////////////////////////////////////////////
class CTestTreeListener : public CListListener
{
CONSTRUCTABLE(CTestTreeListener);
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void SelectionChanged(CUnknown* pcSource, void* pvContext) override
	{
		mpszLog->Append("Selected;");
	}

	void ItemActivated(CUnknown* pcSource, void* pvContext) override
	{
		mpszLog->Append("Activated;");
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The rows' nodes as a string: "0,1,4".
//
//////////////////////////////////////////////////////////////////////////
char* TreeRows(Ptr<CTreeContainer> pTree, CChars* psz)
{
	int32	iRow;

	psz->Clear();
	pTree->UpdateRows();
	for (iRow = 0; iRow < pTree->NumRows(); iRow++)
	{
		if (iRow > 0)
		{
			psz->Append(',');
		}
		psz->Append(pTree->GetRowNode(iRow));
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
// A tree like Solution Explorer's: a project with filters and files.
// Only the nodes at the top and in open nodes are rows.
//
//////////////////////////////////////////////////////////////////////////
void TestTreeNodes(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTreeContainer>		pTree;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		CTestTreeListener		cListener;
		CPointer				pNull;
		CChars					szLog;
		CChars					sz;
		int32					iProject;
		int32					iSource;
		int32					iMap;
		int32					iTile;
		int32					iMain;
		int32					iHeader;
		int32					iTop;
		int32					iHeight;
		SInt32Vec2				sPosition;

		szLog.Init();
		sz.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 200, 200, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Tree Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pTree = OMalloc<CTreeContainer>(pTestWindow, OMalloc<CWindowTheme>(ReadLabelFont()));
		AssertTrue(pTree->AddListener(&cListener));
		pTestWindow->SetContainer(pTree);

		iProject = pTree->AddNode(TREE_NO_NODE, "Project", NULL, 100);
		iSource = pTree->AddNode(iProject, "Source");
		iMap = pTree->AddNode(iSource, "Map");
		iTile = pTree->AddNode(iMap, "TileLayerCel.cpp", NULL, 7);
		iMain = pTree->AddNode(iSource, "Main.cpp");
		iHeader = pTree->AddNode(TREE_NO_NODE, "Header.h");
		AssertInt(6, pTree->NumNodes());
		AssertInt(TREE_NO_NODE, pTree->AddNode(42, "Nowhere"));

		//The nodes.
		AssertInt(0, pTree->GetNodeDepth(iProject));
		AssertInt(2, pTree->GetNodeDepth(iMap));
		AssertInt(iMap, pTree->GetNodeParent(iTile));
		AssertInt(TREE_NO_NODE, pTree->GetNodeParent(iHeader));
		AssertInt(iMap, pTree->GetFirstChild(iSource));
		AssertInt(iMain, pTree->GetNextSibling(iMap));
		AssertInt(TREE_NO_NODE, pTree->GetNextSibling(iMain));
		AssertTrue(pTree->HasChildren(iSource));
		AssertFalse(pTree->HasChildren(iTile));
		AssertLong(7, pTree->GetNodeData(iTile));
		AssertLong(100, pTree->GetNodeData(iProject));
		AssertString("TileLayerCel.cpp", pTree->GetNodeText(iTile, &sz));
		sz.Clear();
		AssertInt(iMain, pTree->FindNode(iSource, "Main.cpp"));
		AssertInt(iHeader, pTree->FindNode(TREE_NO_NODE, "Header.h"));
		AssertInt(TREE_NO_NODE, pTree->FindNode(iSource, "Header.h"));

		//Closed, only the nodes at the top are rows.
		AssertString("0,5", TreeRows(pTree, &sz));
		pTree->SetExpanded(iProject, true);
		AssertString("0,1,5", TreeRows(pTree, &sz));

		//Selecting a node opens the nodes it is in.
		AssertTrue(pTree->SelectNode(iTile));
		AssertTrue(pTree->IsExpanded(iSource));
		AssertTrue(pTree->IsExpanded(iMap));
		AssertString("0,1,2,3,4,5", TreeRows(pTree, &sz));
		AssertInt(iTile, pTree->GetSelectedNode());
		AssertInt(3, pTree->GetSelected());
		AssertInt(3, pTree->FindNodeRow(iTile));

		//Closing a node hides what is in it, and the selection goes to the node it was in.
		pTree->SetExpanded(iSource, false);
		AssertString("0,1,5", TreeRows(pTree, &sz));
		AssertInt(iSource, pTree->GetSelectedNode());
		AssertInt(-1, pTree->FindNodeRow(iTile));

		//Map stayed open: opening Source shows it open.
		pTree->Toggle(iSource);
		AssertString("0,1,2,3,4,5", TreeRows(pTree, &sz));

		//Activating a node with children opens or closes it, and the listeners are told; a node without is only told.
		szLog.Clear();
		pTree->SelectNode(iMap);
		pTree->Activate();
		AssertFalse(pTree->IsExpanded(iMap));
		AssertString("0,1,2,4,5", TreeRows(pTree, &sz));
		pTree->SelectNode(iMain);
		pTree->Activate();
		AssertString("0,1,2,4,5", TreeRows(pTree, &sz));
		AssertString("Selected;Activated;Selected;Activated;", szLog.Text());

		//Left closes an open node or goes to the node it is in; right opens a closed node or goes into an open one.
		pTree->SelectNode(iMain);
		pTree->Left();
		AssertInt(iSource, pTree->GetSelectedNode());
		pTree->Left();
		AssertFalse(pTree->IsExpanded(iSource));
		AssertInt(iSource, pTree->GetSelectedNode());
		pTree->Left();
		AssertInt(iProject, pTree->GetSelectedNode());
		pTree->Right();
		AssertInt(iSource, pTree->GetSelectedNode());
		pTree->Right();
		AssertTrue(pTree->IsExpanded(iSource));
		pTree->Right();
		AssertInt(iMap, pTree->GetSelectedNode());

		//A press on a chevron opens or closes its node; on the name it only selects.
		pTree->ExpandAll();
		AssertString("0,1,2,3,4,5", TreeRows(pTree, &sz));
		pTestWindow->Layout();
		sPosition = pTree->GetPosition();
		AssertTrue(pTree->GetTrackBounds(false, 1, &iTop, &iHeight));
		AssertTrue(pTree->IsOverChevron(1, 16 + 2));
		AssertFalse(pTree->IsOverChevron(1, 2));
		AssertFalse(pTree->IsOverChevron(3, 48 + 2));
		pTree->ActivateComponent(16 + 2, iTop - sPosition.y + 2);
		AssertInt(iSource, pTree->GetSelectedNode());
		AssertFalse(pTree->IsExpanded(iSource));
		AssertString("0,1,5", TreeRows(pTree, &sz));
		pTree->ActivateComponent(100, iTop - sPosition.y + 2);
		AssertFalse(pTree->IsExpanded(iSource));
		pTree->CollapseAll();
		AssertString("0,5", TreeRows(pTree, &sz));

		pTree->RemoveAllNodes();
		AssertInt(0, pTree->NumNodes());
		AssertInt(0, pTree->NumRows());
		AssertInt(TREE_NO_NODE, pTree->GetSelectedNode());

		pTree->RemoveListener(&cListener);
		pTree = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		sz.Kill();
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTree(void)
{
	BeginTests();
	ObjectsInit();

	TestTreeNodes();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

