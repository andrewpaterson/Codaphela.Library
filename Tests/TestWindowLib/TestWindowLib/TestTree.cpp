#include "BaseLib/GlobalMemory.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/TreeContainer.h"
#include "WindowLib/ScrollPane.h"
#include "WindowLib/JewelTheme.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


Ptr<CWindowTheme> CreateTestTheme(Ptr<CFont> pFont);


Ptr<CFont> ReadLabelFont(void);


Ptr<CScrollBarImages> CreateScrollBarImages(void);
Ptr<CWindowTheme> CreateScrollPaneTheme(Ptr<CScrollBarImages> pImages);


//////////////////////////////////////////////////////////////////////////
//
// Logs the selection changing and nodes being activated.
//
//////////////////////////////////////////////////////////////////////////
class CTestTreeListener
{
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void SelectionChanged(void)
	{
		mpszLog->Append("Selected;");
	}

	void ItemActivated(void)
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
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTreeContainer>		pTree;
		Ptr<CFont>				pFont;
		size					uiStart;
		size					uiLength;
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
		Ptr<CWindowTheme>		pTheme;

		szLog.Init();
		sz.Init();
		cListener.Init(&szLog);
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 200, 200, (char*)NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Tree Window", &pNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pTheme = CreateTestTheme(ReadLabelFont());
		CJewelTheme::SetImages(pTheme, pTestWindow);  //Its chevrons.
		pTree = OMalloc<CTreeContainer>(pTestWindow, pTheme);
		pTree->GetSelectionEvent()->Add(&cListener, &CTestTreeListener::SelectionChanged);
		pTree->GetActivationEvent()->Add(&cListener, &CTestTreeListener::ItemActivated);
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

		//A node's name is in the theme's font unless it is given one; NULL gives it back.
		pFont = ReadLabelFont();
		AssertTrue(pTree->GetNodeFont(iMain).IsNull());
		pTree->SetNodeFont(iMain, pFont);
		AssertTrue(pTree->GetNodeFont(iMain) == pFont);
		AssertTrue(pTree->GetNodeFont(iTile).IsNull());
		pTree->SetNodeFont(iMain, NULL);
		AssertTrue(pTree->GetNodeFont(iMain).IsNull());
		pTree->SetNodeFont(42, pFont);

		//Parts of a node's name can have fonts of their own: a node's ranges are added together, in the order of its
		//text, without overlapping, and must be in the name.  (Main.cpp's name is 8 characters.)
		AssertSize(0, pTree->NumNodeRanges(iMain));
		AssertFalse(pTree->GetNodeRange(iMain, 0, &uiStart, &uiLength));
		AssertFalse(pTree->AddNodeRange(iMain, 0, 0, pFont));
		AssertFalse(pTree->AddNodeRange(iMain, 0, 9, pFont));
		AssertFalse(pTree->AddNodeRange(iMain, 8, 1, pFont));
		AssertFalse(pTree->AddNodeRange(iMain, 0, 4, NULL));
		AssertFalse(pTree->AddNodeRange(42, 0, 4, pFont));
		AssertTrue(pTree->AddNodeRange(iMain, 0, 4, pFont));
		AssertFalse(pTree->AddNodeRange(iMain, 3, 2, pFont));
		AssertTrue(pTree->AddNodeRange(iMain, 5, 3, pFont));
		AssertSize(2, pTree->NumNodeRanges(iMain));
		AssertTrue(pTree->GetNodeRange(iMain, 1, &uiStart, &uiLength));
		AssertSize(5, uiStart);
		AssertSize(3, uiLength);
		AssertTrue(pTree->GetNodeRangeFont(iMain, 1) == pFont);
		AssertTrue(pTree->GetNodeRangeFont(iMain, 2).IsNull());
		AssertFalse(pTree->GetNodeRange(iMain, 2, &uiStart, &uiLength));

		//Another node's ranges; after them the first's can't be added to.
		AssertTrue(pTree->AddNodeRange(iTile, 0, 1, pFont));
		AssertFalse(pTree->AddNodeRange(iMain, 4, 1, pFont));
		AssertSize(1, pTree->NumNodeRanges(iTile));
		AssertSize(2, pTree->NumNodeRanges(iMain));
		AssertSize(0, pTree->NumNodeRanges(iProject));
		pFont = NULL;

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
		AssertTrue(pTree->GetRowBounds(1, &iTop, &iHeight));
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

		pTree->GetSelectionEvent()->Remove(&cListener);
		pTree->GetActivationEvent()->Remove(&cListener);
		pTree = NULL;
		pTheme = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		sz.Kill();
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The tree only has components for the rows in view.  Each shows its
// row's node: a chevron if the node has children, its icon, and its name
// in a label, or in a label for each part in a font of its own.  The
// tree is as wide as its widest row, which it works out from the fonts:
// the same as the row's labels are laid out to.
//
//////////////////////////////////////////////////////////////////////////
void TestTreeRowComponents(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTreeContainer>		pTree;
		Ptr<CTreeRow>			pRow;
		Ptr<CFont>				pFont;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		CPointer				pNull;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CImageCel>			pIcon;
		int32					iProject;
		int32					iSource;
		int32					iMain;
		int32					iHeader;
		int32					iTop;
		int32					iHeight;
		int32					iWidth;
		int32					iWidest;
		int32					i;
		size					uiName;
		const char*				aszNames[] = { "Main.cpp", "a b  c", "12: int F(void) { return G(1, 2) + 3; }", "\tTabbed\ttext", "", "x" };

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 300, 200, (char*)NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Tree Rows Window", &pNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFont = ReadLabelFont();
		pTheme = CreateTestTheme(pFont);
		CJewelTheme::SetImages(pTheme, pTestWindow);
		pIcon = pTheme->GetImage(TI_DownChevron);
		pTree = OMalloc<CTreeContainer>(pTestWindow, pTheme);
		pTestWindow->SetContainer(pTree);

		//Before it is laid out the tree has no components for its rows, and doesn't know where they are.
		iProject = pTree->AddNode(TREE_NO_NODE, "Project");
		iSource = pTree->AddNode(iProject, "Source", pIcon);
		iMain = pTree->AddNode(iSource, "Main.cpp");
		iHeader = pTree->AddNode(TREE_NO_NODE, "Header.h", pIcon);
		AssertTrue(pTree->AddNodeRange(iMain, 0, 4, pFont));
		AssertTrue(pTree->AddNodeRange(iMain, 5, 3, pFont));
		AssertSize(0, pTree->NumRowComponents());
		AssertFalse(pTree->GetRowBounds(0, &iTop, &iHeight));
		AssertTrue(pTree->GetRowComponent(0).IsNull());

		//Laid out, closed: a row for each node at the top, each with its component.
		pTestWindow->Layout();
		AssertInt(2, pTree->NumRows());
		AssertTrue(pTree->GetRowBounds(1, &iTop, &iHeight));
		AssertInt(pTree->GetPosition().y + iHeight, iTop);
		pRow = pTree->GetRowComponent(0);
		AssertTrue(pRow.IsNotNull());
		AssertInt(iProject, pRow->GetNode());
		AssertTrue(pRow->HasChevron());
		AssertFalse(pRow->HasIcon());
		AssertSize(1, pRow->NumLabels());
		AssertString("Project", pRow->GetLabel(0)->GetText());
		AssertInt(pTree->GetPosition().y, pRow->GetPosition().y);
		AssertInt(iHeight, pRow->GetActualSize().y);
		pRow = pTree->GetRowComponent(1);
		AssertInt(iHeader, pRow->GetNode());
		AssertFalse(pRow->HasChevron());
		AssertTrue(pRow->HasIcon());
		AssertString("Header.h", pRow->GetLabel(0)->GetText());
		AssertInt(iTop, pRow->GetPosition().y);
		AssertTrue(pTree->GetRowComponent(2).IsNull());

		//Opened, the rows below move down: each row's component shows the node that is now its row's.  A name with
		//ranges has a label for each range and for the text between them.
		pTree->ExpandAll();
		pTestWindow->Layout();
		AssertInt(4, pTree->NumRows());
		AssertInt(iSource, pTree->GetRowComponent(1)->GetNode());
		AssertTrue(pTree->GetRowComponent(1)->HasChevron());
		AssertTrue(pTree->GetRowComponent(1)->HasIcon());
		pRow = pTree->GetRowComponent(2);
		AssertInt(iMain, pRow->GetNode());
		AssertSize(3, pRow->NumLabels());
		AssertString("Main", pRow->GetLabel(0)->GetText());
		AssertString(".", pRow->GetLabel(1)->GetText());
		AssertString("cpp", pRow->GetLabel(2)->GetText());
		AssertTrue(pRow->GetLabel(3).IsNull());
		AssertInt(iHeader, pTree->GetRowComponent(3)->GetNode());
		AssertString("Header.h", pTree->GetRowComponent(3)->GetLabel(0)->GetText());

		//Each label is where the one before it ends, the first after the indent (two deep), the chevron's gap and no icon.
		AssertInt(pRow->GetLabel(0)->GetPosition().x + pRow->GetLabel(0)->GetActualSize().x, pRow->GetLabel(1)->GetPosition().x);
		AssertInt(pRow->GetLabel(1)->GetPosition().x + pRow->GetLabel(1)->GetActualSize().x, pRow->GetLabel(2)->GetPosition().x);
		AssertInt(pTree->GetPosition().x + 2 * pTheme->GetMetric(TM_TreeIndent) + pTheme->GetMetric(TM_TreeChevronWidth), pRow->GetLabel(0)->GetPosition().x);

		//The tree is as wide as its widest row needs and as tall as its rows: what it works out from the fonts is what
		//the rows' components need.
		iWidest = 0;
		for (i = 0; i < pTree->NumRows(); i++)
		{
			pRow = pTree->GetRowComponent(i);
			pRow->SetRequiredSize();
			iWidth = pRow->GetRequiredSize().x;
			if (iWidth > iWidest)
			{
				iWidest = iWidth;
			}
		}
		AssertInt(iWidest, pTree->GetDesiredSize().x);
		AssertInt(4 * iHeight, pTree->GetDesiredSize().y);

		//Closed again, the components of the rows that went show no node, and are kept for when rows come back.
		pTree->CollapseAll();
		pTestWindow->Layout();
		AssertInt(2, pTree->NumRows());
		AssertInt(iHeader, pTree->GetRowComponent(1)->GetNode());
		AssertTrue(pTree->GetRowComponent(2).IsNull());
		AssertSize(pTree->NumRowComponents() + 2, pTree->NumComponents());

		//One node of each name, alone: as wide as its component needs, whatever is in the name.
		for (uiName = 0; uiName < sizeof(aszNames) / sizeof(aszNames[0]); uiName++)
		{
			pTree->RemoveAllNodes();
			pTree->AddNode(TREE_NO_NODE, aszNames[uiName], (uiName & 1) ? pIcon : Ptr<CImageCel>());
			pTestWindow->Layout();
			pRow = pTree->GetRowComponent(0);
			AssertTrue(pRow.IsNotNull());
			pRow->SetRequiredSize();
			AssertInt(pRow->GetRequiredSize().x, pTree->GetDesiredSize().x);
		}

		pRow = NULL;
		pIcon = NULL;
		pFont = NULL;
		pTree = NULL;
		pTheme = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A hundred thousand nodes, all open, in a scroll pane: the tree is as
// tall as all of them, but has no more components than the rows the
// scroll pane shows, however far it is scrolled.  Rows are found, shown
// and selected by where their heights put them.
//
//////////////////////////////////////////////////////////////////////////
void TestTreeManyRows(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CScrollPane>		pScrollPane;
		Ptr<CTreeContainer>		pTree;
		Ptr<CTreeRow>			pRow;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		CPointer				pNull;
		Ptr<CWindowTheme>		pTheme;
		CChars					sz;
		int32					iRoot;
		int32					iTop;
		int32					iHeight;
		int32					iShown;
		int32					i;
		size					uiComponents;
		SInt32Vec2				sPosition;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 300, 200, (char*)NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Tree Many Rows Window", &pNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pTheme = CreateTestTheme(ReadLabelFont());
		CJewelTheme::SetImages(pTheme, pTestWindow);
		pScrollPane = OMalloc<CScrollPane>(pTestWindow, CreateScrollPaneTheme(CreateScrollBarImages()));
		pTestWindow->SetContainer(pScrollPane);
		pTree = OMalloc<CTreeContainer>(pTestWindow, pTheme);
		pTree->SetAlignment(CSH_Stretch, CSV_Top);
		pScrollPane->SetContent(pTree);

		sz.Init();
		iRoot = pTree->AddNode(TREE_NO_NODE, "Root");
		for (i = 1; i < 100000; i++)
		{
			sz.Clear();
			sz.Append("Node ");
			sz.Append(i);
			pTree->AddNode(iRoot, sz.Text(), NULL, i);
		}
		pTree->SetExpanded(iRoot, true);
		pTestWindow->Layout();
		AssertInt(100000, pTree->NumRows());
		AssertTrue(pTree->GetRowBounds(0, &iTop, &iHeight));
		AssertInt(100000 * iHeight, pTree->GetDesiredSize().y);

		//As many components as rows the 200 high window shows (made a few at a time), and no more.
		iShown = (200 + iHeight - 1) / iHeight;
		uiComponents = pTree->NumRowComponents();
		AssertTrue((uiComponents >= (size)iShown) && (uiComponents < (size)iShown + 8));
		AssertSize(uiComponents + 2, pTree->NumComponents());
		AssertInt(0, pTree->GetRowComponent(0)->GetNode());
		AssertString("Node 1", pTree->GetRowComponent(1)->GetLabel(0)->GetText());
		AssertTrue(pTree->GetRowComponent(iShown - 1).IsNotNull());
		AssertTrue(pTree->GetRowComponent(iShown).IsNull());
		AssertTrue(pTree->GetRowComponent(99999).IsNull());

		//The last row shown: the same components now show the last rows, where their heights put them.
		AssertTrue(pTree->Select(99999));
		pTree->ShowRow(99999);
		pTestWindow->Layout();
		AssertInt(100000 * iHeight - 200, pScrollPane->GetScrollPosition().y);
		AssertSize(uiComponents, pTree->NumRowComponents());
		AssertSize(uiComponents + 2, pTree->NumComponents());
		pRow = pTree->GetRowComponent(99999);
		AssertTrue(pRow.IsNotNull());
		AssertString("Node 99999", pRow->GetLabel(0)->GetText());
		AssertInt(200 - iHeight, pRow->GetPosition().y);
		AssertTrue(pTree->GetRowBounds(99999, &iTop, &iHeight));
		AssertInt(200 - iHeight, iTop);
		AssertTrue(pTree->GetRowComponent(0).IsNull());
		AssertTrue(pTree->GetRowComponent(99999 - iShown).IsNull());
		AssertTrue(pTree->GetRowComponent(99999 - iShown + 2).IsNotNull());

		//The row at a point is found from its height, and a press there selects it.
		sPosition = pTree->GetPosition();
		AssertInt(99999, pTree->FindRowAt(50, 200 - 1));
		AssertInt(99998, pTree->FindRowAt(50, 200 - iHeight - 1));
		AssertInt(-1, pTree->FindRowAt(50, 200));
		pTree->ActivateComponent(50, 200 - iHeight - 1 - sPosition.y);
		AssertInt(99998, pTree->GetSelected());
		AssertLong(99998, pTree->GetNodeData(pTree->GetSelectedNode()));

		//The keys move the selection over rows that have no components until they are scrolled to.
		pTree->DocumentHome();
		pTestWindow->Layout();
		AssertInt(0, pTree->GetSelected());
		AssertInt(0, pScrollPane->GetScrollPosition().y);
		AssertInt(0, pTree->GetRowComponent(0)->GetNode());
		pTree->PageDown();
		pTree->PageDown();
		pTestWindow->Layout();
		AssertTrue(pTree->GetSelected() > iShown);
		AssertTrue(pTree->GetRowComponent(pTree->GetSelected()).IsNotNull());
		AssertInt(pTree->GetSelected(), pTree->GetRowComponent(pTree->GetSelected())->GetNode());
		AssertSize(uiComponents, pTree->NumRowComponents());

		//Closed, there is one row, and the rest of the components show no node.
		pTree->SetExpanded(iRoot, false);
		pTestWindow->Layout();
		AssertInt(1, pTree->NumRows());
		AssertInt(iRoot, pTree->GetSelectedNode());
		AssertInt(iHeight, pTree->GetDesiredSize().y);
		AssertInt(iRoot, pTree->GetRowComponent(0)->GetNode());
		AssertTrue(pTree->GetRowComponent(1).IsNull());
		AssertSize(uiComponents, pTree->NumRowComponents());

		sz.Kill();
		pRow = NULL;
		pTree = NULL;
		pScrollPane = NULL;
		pTheme = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Typing finds a row of a tree by its node's text: only the nodes that
// can be seen are found.
//
//////////////////////////////////////////////////////////////////////////
void TestTreeTypeToFind(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTreeContainer>		pTree;
		Ptr<CScrollPane>		pScrollPane;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		CPointer				pNull;
		CChars					sz;
		int32					iProject;
		int32					iSource;
		int32					iMap;
		int32					iMain;
		int32					iHeader;

		sz.Init();
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 200, 200, (char*)NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Tree Find Window", &pNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pTheme = CreateTestTheme(ReadLabelFont());
		CJewelTheme::SetImages(pTheme, pTestWindow);
		pScrollPane = OMalloc<CScrollPane>(pTestWindow, CreateScrollPaneTheme(CreateScrollBarImages()));
		pTestWindow->SetContainer(pScrollPane);
		pTree = OMalloc<CTreeContainer>(pTestWindow, pTheme);
		pScrollPane->SetContent(pTree);

		iProject = pTree->AddNode(TREE_NO_NODE, "Project");
		iSource = pTree->AddNode(iProject, "Source");
		iMap = pTree->AddNode(iSource, "Map.cpp");
		iMain = pTree->AddNode(iSource, "Main.cpp");
		iHeader = pTree->AddNode(iProject, "Header.h");
		pTree->SetExpanded(iProject, true);
		pTestWindow->Layout();
		pTestWindow->GetFocus()->SetFocus(pTree);

		AssertTrue(pTree->GetRowText(1, &sz));
		AssertString("Source", sz.Text());
		AssertFalse(pTree->GetRowText(3, &sz));

		//Source is closed, so Main.cpp in it isn't a row and isn't found: "m" finds nothing but "h" finds the header.
		pTree->SetTypeToFind(true);
		pTree->SelectNode(iProject);
		pTestWindow->InputPrintable('m', true);
		AssertTrue(pTree->IsFinding());
		AssertFalse(pTree->IsFindMatched());
		AssertInt(iProject, pTree->GetSelectedNode());
		pTestWindow->InputCommand(CIC_Backspace, true);
		AssertFalse(pTree->IsFinding());
		pTestWindow->InputPrintable('h', true);
		AssertTrue(pTree->IsFindMatched());
		AssertInt(iHeader, pTree->GetSelectedNode());
		AssertTrue(pScrollPane->GetNote() == pTree->GetFindNote());
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pTree->IsFinding());

		//Open, its files are found, the next with down.
		pTree->SetExpanded(iSource, true);
		pTree->SelectNode(iProject);
		pTestWindow->InputPrintable('m', true);
		AssertInt(iMap, pTree->GetSelectedNode());
		pTestWindow->InputPrintable('a', true);
		AssertInt(iMap, pTree->GetSelectedNode());
		pTestWindow->InputCommand(CIC_Down, true);
		AssertInt(iMain, pTree->GetSelectedNode());
		pTestWindow->InputPrintable('i', true);
		AssertString("mai", pTree->GetFindText());
		AssertInt(iMain, pTree->GetSelectedNode());
		pTree->StopFinding();
		AssertTrue(pScrollPane->GetNote().IsNull());

		sz.Kill();
		pTree = NULL;
		pScrollPane = NULL;
		pTheme = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}
	pNativeFactory = NULL;
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
	TestTreeRowComponents();
	TestTreeManyRows();
	TestTreeTypeToFind();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

