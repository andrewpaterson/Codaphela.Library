#include <stdlib.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "SupportLib/PNGWriter.h"
#include "WindowLib/Canvas.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/JewelColours.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/NodeEditor.h"
#include "WindowLib/NumberField.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"


Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);
void			MoveTo(Ptr<CWindow> pWindow, SInt32Vec2 sPosition);


#define AssertLog(e)	AssertString(e, cListener.mszLog.Text()); cListener.mszLog.Clear()


//Logs what the editor asks and tells, by the nodes' titles.
class CTestNodeEditorListener : public CNodeEditorListener
{
public:
	CChars	mszLog;
	bool	mbAllow;

	void Log(const char* szWhat, CNodeBox* pcFrom, size uiOutput, CNodeBox* pcTo, size uiInput)
	{
		mszLog.Append(szWhat);
		mszLog.Append(":");
		mszLog.Append(pcFrom->GetTitle());
		mszLog.Append((int32)uiOutput);
		mszLog.Append(">");
		mszLog.Append(pcTo->GetTitle());
		mszLog.Append((int32)uiInput);
		mszLog.Append(";");
	}

	bool CanLink(CNodeBox* pcFrom, size uiOutput, CNodeBox* pcTo, size uiInput) override
	{
		Log("Can", pcFrom, uiOutput, pcTo, uiInput);
		return mbAllow;
	}

	void Linked(CNodeBox* pcFrom, size uiOutput, CNodeBox* pcTo, size uiInput) override { Log("Linked", pcFrom, uiOutput, pcTo, uiInput); }
	void Unlinked(CNodeBox* pcFrom, size uiOutput, CNodeBox* pcTo, size uiInput) override { Log("Unlinked", pcFrom, uiOutput, pcTo, uiInput); }
	void NodesMoved(void) override { mszLog.Append("Moved;"); }
	void DeleteSelected(void) override { mszLog.Append("Delete;"); }
};


//The editor a test works on and its three nodes: Noise (an output), Ramp (an input with a value, and an output) and Out
//(an input), left to right.
struct STestNodeEditor
{
	CNodeEditor*	pcEditor;
	CNodeBox*		pcNoise;
	CNodeBox*		pcRamp;
	CNodeBox*		pcOut;
};


//////////////////////////////////////////////////////////////////////////
//
// The editor filling a window, with the three nodes.  The window holds
// them all.
//
//////////////////////////////////////////////////////////////////////////
static void CreateTestNodeEditor(Ptr<CWindow> pWindow, STestNodeEditor* psTest)
{
	Ptr<CFillContainer>		pFill;
	Ptr<CWindowTheme>		pTheme;
	Ptr<CNodeEditor>		pEditor;
	Ptr<CNodeBox>			pNode;
	Ptr<CNumberField>		pField;

	pTheme = CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow));
	pFill = OMalloc<CFillContainer>(pWindow);
	pWindow->SetContainer(pFill);
	pEditor = OMalloc<CNodeEditor>(pWindow, pTheme);
	pFill->AddComponent(pEditor);
	psTest->pcEditor = &pEditor;

	pNode = OMalloc<CNodeBox>(pWindow, pTheme, "Noise", JEWEL_BLUE);
	pNode->AddOutput("Colour", JEWEL_TEXT);
	pNode->AddRow(OMalloc<CNumberField>(pWindow, pTheme, "Scale", 4.0));
	pNode->SetNodePosition(20, 30);
	pEditor->AddNode(pNode);
	psTest->pcNoise = &pNode;

	pNode = OMalloc<CNodeBox>(pWindow, pTheme, "Ramp", JEWEL_GREEN);
	pNode->AddOutput("Colour", JEWEL_TEXT);
	pField = OMalloc<CNumberField>(pWindow, pTheme, "Texture", 0.5);
	pField->SetRange(0.0, 1.0);
	pNode->AddInput("Texture", JEWEL_TEXT, pField);
	pNode->SetNodePosition(260, 60);
	pEditor->AddNode(pNode);
	psTest->pcRamp = &pNode;

	pNode = OMalloc<CNodeBox>(pWindow, pTheme, "Out", JEWEL_RED);
	pNode->AddInput("Surface", JEWEL_TEXT);
	pNode->SetNodePosition(500, 40);
	pEditor->AddNode(pNode);
	psTest->pcOut = &pNode;

	pWindow->Layout();
}


//////////////////////////////////////////////////////////////////////////
//
// The left button pressed at sFrom, moved to sTo and let go, in the
// window.
//
//////////////////////////////////////////////////////////////////////////
static void DragTestNodeEditor(Ptr<CWindow> pWindow, SInt32Vec2 sFrom, SInt32Vec2 sTo)
{
	MoveTo(pWindow, sFrom);
	pWindow->InputCommand(CIC_ActivateComponent, true);
	MoveTo(pWindow, sTo);
	pWindow->Layout();
	pWindow->InputCommand(CIC_ExecuteComponent, true);
	pWindow->Layout();
}


//////////////////////////////////////////////////////////////////////////
//
// The middle of a node's title, where pressing takes hold of the node.
//
//////////////////////////////////////////////////////////////////////////
static SInt32Vec2 TestNodeEditorTitle(CNodeBox* pcNode)
{
	SInt32Vec2	sPosition;

	sPosition = pcNode->GetPosition();
	return SInt32Vec2(sPosition.x + pcNode->GetActualSize().x / 2, sPosition.y + pcNode->GetHeaderHeight() / 2);
}


//////////////////////////////////////////////////////////////////////////
//
// A node is where it says, from the editor's origin moved by the pan;
// as wide as a node box and as tall as its rows need; its sockets are on
// its body's edges.  Home puts the middle of the nodes in the middle.
//
//////////////////////////////////////////////////////////////////////////
void TestNodeEditorLayout(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>		pWindow;
		STestNodeEditor		sTest;
		CPointer			pNull;
		SInt32Vec2			sPosition;
		SInt32Vec2			sSize;
		SInt32Vec2			sSocket;
		SInt32Vec2			sFirst;
		SInt32Vec2			sLast;
		SComponentInputEvent	sEvent;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 800, 400, (char*)NULL);
		pWindow = OMalloc<CWindow>("Node Editor Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		CreateTestNodeEditor(pWindow, &sTest);

		AssertSize(3, sTest.pcEditor->NumNodes());
		sSize = sTest.pcEditor->GetActualSize();
		AssertInt(800, sSize.x);
		AssertInt(400, sSize.y);

		sPosition = sTest.pcNoise->GetPosition();
		AssertInt(20, sPosition.x);
		AssertInt(30, sPosition.y);
		sSize = sTest.pcNoise->GetActualSize();
		AssertInt(NODE_BOX_WIDTH, sSize.x);
		AssertTrue((sSize.y > 50) && (sSize.y < 120));  //A title, an output and a number.
		AssertTrue(sTest.pcOut->GetActualSize().y < sSize.y);  //A title and an input.

		//The output is on the right edge of the body, under the header; Ramp's input is on its left edge, under its output.
		AssertSize(1, sTest.pcNoise->NumSockets(false));
		AssertSize(0, sTest.pcNoise->NumSockets(true));
		sSocket = sTest.pcNoise->GetSocketPosition(false, 0);
		AssertInt(20 + NODE_BOX_WIDTH - NODE_BOX_SOCKET_RADIUS, sSocket.x);
		AssertTrue((sSocket.y > 30 + sTest.pcNoise->GetHeaderHeight()) && (sSocket.y < 30 + sSize.y));
		sFirst = sTest.pcRamp->GetSocketPosition(false, 0);
		sLast = sTest.pcRamp->GetSocketPosition(true, 0);
		AssertInt(260 + NODE_BOX_SOCKET_RADIUS, sLast.x);
		AssertTrue(sLast.y > sFirst.y);

		//The topmost node at a point; none on the background.
		AssertTrue(&sTest.pcEditor->FindNodeAt(30, 40) == sTest.pcNoise);
		AssertTrue(sTest.pcEditor->FindNodeAt(230, 40).IsNull());

		//The pan moves them all.
		sTest.pcEditor->SetPan(15, -10);
		pWindow->Layout();
		sPosition = sTest.pcNoise->GetPosition();
		AssertInt(35, sPosition.x);
		AssertInt(20, sPosition.y);
		sPosition = sTest.pcOut->GetPosition();
		AssertInt(515, sPosition.x);
		AssertInt(30, sPosition.y);

		//Home: the nodes go from 20 to 500 + a box's width across, so their middle is 355 and the editor's is 400.
		memset(&sEvent, 0, sizeof(sEvent));
		sTest.pcEditor->HomeAction(&sEvent);
		pWindow->Layout();
		AssertInt(45, sTest.pcEditor->GetPan().x);
		AssertInt(65, sTest.pcNoise->GetPosition().x);
		AssertInt(400 - (sTest.pcNoise->GetPosition().x + NODE_BOX_WIDTH / 2), (sTest.pcOut->GetPosition().x + NODE_BOX_WIDTH / 2) - 400);

		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Links made by the editor's functions: an input takes one, an output
// feeds many, a node isn't linked to itself, an input's value gives way
// to its name while it's linked, and a node goes with its links.
//
//////////////////////////////////////////////////////////////////////////
void TestNodeEditorLinks(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>		pWindow;
		STestNodeEditor		sTest;
		CPointer			pNull;
		SNodeLink*			psLink;
		int32				iUnlinked;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 800, 400, (char*)NULL);
		pWindow = OMalloc<CWindow>("Node Editor Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		CreateTestNodeEditor(pWindow, &sTest);

		AssertSize(0, sTest.pcEditor->NumLinks());
		AssertFalse(sTest.pcRamp->IsInputLinked(0));
		iUnlinked = sTest.pcRamp->GetSocketPosition(true, 0).y;

		AssertTrue(sTest.pcEditor->Link(sTest.pcNoise, 0, sTest.pcRamp, 0));
		AssertTrue(sTest.pcEditor->Link(sTest.pcNoise, 0, sTest.pcOut, 0));
		AssertSize(2, sTest.pcEditor->NumLinks());
		AssertTrue(sTest.pcRamp->IsInputLinked(0));
		pWindow->Layout();
		AssertTrue(abs(sTest.pcRamp->GetSocketPosition(true, 0).y - iUnlinked) <= 4);  //The name's row is where the value's was, or near.

		//Another link to Out's input takes the first's place.
		AssertTrue(sTest.pcEditor->Link(sTest.pcRamp, 0, sTest.pcOut, 0));
		AssertSize(2, sTest.pcEditor->NumLinks());
		psLink = sTest.pcEditor->FindLinkTo(sTest.pcOut, 0);
		AssertTrue(psLink->pcFrom == sTest.pcRamp);

		//Not to itself, nor to or from sockets there aren't.
		AssertFalse(sTest.pcEditor->Link(sTest.pcRamp, 0, sTest.pcRamp, 0));
		AssertFalse(sTest.pcEditor->Link(sTest.pcNoise, 1, sTest.pcOut, 0));
		AssertFalse(sTest.pcEditor->Link(sTest.pcNoise, 0, sTest.pcOut, 1));
		AssertSize(2, sTest.pcEditor->NumLinks());

		AssertTrue(sTest.pcEditor->Unlink(sTest.pcRamp, 0));
		AssertFalse(sTest.pcEditor->Unlink(sTest.pcRamp, 0));
		AssertFalse(sTest.pcRamp->IsInputLinked(0));
		AssertSize(1, sTest.pcEditor->NumLinks());

		//Ramp goes, and its link to Out with it.
		AssertTrue(sTest.pcEditor->RemoveNode(sTest.pcRamp));
		AssertSize(2, sTest.pcEditor->NumNodes());
		AssertSize(0, sTest.pcEditor->NumLinks());
		AssertFalse(sTest.pcOut->IsInputLinked(0));

		sTest.pcEditor->RemoveAllNodes();
		AssertSize(0, sTest.pcEditor->NumNodes());

		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// With the pointer: a link dragged from an output to an input is made if
// the listener allows it; an input's link is taken hold of by its end
// and removed by letting go over nothing, or put on another input; a
// node is selected and moved by its title; a box selects what it
// touches; the background selects nothing.
//
//////////////////////////////////////////////////////////////////////////
void TestNodeEditorPointer(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "NodeEditor";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);

	{
		Ptr<CWindow>				pWindow;
		STestNodeEditor				sTest;
		CTestNodeEditorListener		cListener;
		CPointer					pNull;
		SInt32Vec2					sFrom;
		SInt32Vec2					sTo;
		SInt32Vec2					sNode;
		SComponentInputEvent		sEvent;

		cListener.mszLog.Init();
		cListener.mbAllow = true;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 800, 400, szDirectory);
		pWindow = OMalloc<CWindow>("Node Editor Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		CreateTestNodeEditor(pWindow, &sTest);
		sTest.pcEditor->SetListener(&cListener);

		//Noise's output dragged to Ramp's input.
		sFrom = sTest.pcNoise->GetSocketPosition(false, 0);
		sTo = sTest.pcRamp->GetSocketPosition(true, 0);
		MoveTo(pWindow, sFrom);
		pWindow->InputCommand(CIC_ActivateComponent, true);
		AssertInt(NED_Link, sTest.pcEditor->GetDrag());
		MoveTo(pWindow, sTo);
		pWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertInt(NED_None, sTest.pcEditor->GetDrag());
		AssertSize(1, sTest.pcEditor->NumLinks());
		AssertTrue(sTest.pcRamp->IsInputLinked(0));
		AssertLog("Can:Noise0>Ramp0;Linked:Noise0>Ramp0;");
		pWindow->Layout();

		//Dragged the other way, from an input to an output: Out's input to Ramp's output.
		DragTestNodeEditor(pWindow, sTest.pcOut->GetSocketPosition(true, 0), sTest.pcRamp->GetSocketPosition(false, 0));
		AssertSize(2, sTest.pcEditor->NumLinks());
		AssertLog("Can:Ramp0>Out0;Linked:Ramp0>Out0;");

		//NODE_EDITOR_PICTURE=file saves the window as it now is, to look at.
		pWindow->Paint();
		if (getenv("NODE_EDITOR_PICTURE") != NULL)
		{
			SavePNG(pWindow->GetCanvas()->GetImageOrNull(), getenv("NODE_EDITOR_PICTURE"));
		}

		//Not allowed, nothing changes; let go over an output or over nothing, nothing's asked.
		cListener.mbAllow = false;
		DragTestNodeEditor(pWindow, sTest.pcNoise->GetSocketPosition(false, 0), sTest.pcOut->GetSocketPosition(true, 0));
		AssertSize(2, sTest.pcEditor->NumLinks());
		AssertTrue(sTest.pcEditor->FindLinkTo(sTest.pcOut, 0)->pcFrom == sTest.pcRamp);
		AssertLog("Can:Noise0>Out0;");
		cListener.mbAllow = true;
		DragTestNodeEditor(pWindow, sTest.pcNoise->GetSocketPosition(false, 0), sTest.pcRamp->GetSocketPosition(false, 0));
		DragTestNodeEditor(pWindow, sTest.pcNoise->GetSocketPosition(false, 0), SInt32Vec2(240, 300));
		AssertSize(2, sTest.pcEditor->NumLinks());
		AssertLog("");

		//Allowed, it takes the place of Out's link.
		DragTestNodeEditor(pWindow, sTest.pcNoise->GetSocketPosition(false, 0), sTest.pcOut->GetSocketPosition(true, 0));
		AssertSize(2, sTest.pcEditor->NumLinks());
		AssertTrue(sTest.pcEditor->FindLinkTo(sTest.pcOut, 0)->pcFrom == sTest.pcNoise);
		AssertLog("Can:Noise0>Out0;Unlinked:Ramp0>Out0;Linked:Noise0>Out0;");

		//Ramp's input's link taken by its end and let go over nothing: gone.
		DragTestNodeEditor(pWindow, sTest.pcRamp->GetSocketPosition(true, 0), SInt32Vec2(240, 300));
		AssertSize(1, sTest.pcEditor->NumLinks());
		AssertFalse(sTest.pcRamp->IsInputLinked(0));
		AssertLog("Unlinked:Noise0>Ramp0;");

		//A node pressed by its title is the selected one, and goes with the pointer.
		AssertSize(0, sTest.pcEditor->NumSelected());
		sNode = sTest.pcRamp->GetNodePosition();
		sFrom = TestNodeEditorTitle(sTest.pcRamp);
		DragTestNodeEditor(pWindow, sFrom, SInt32Vec2(sFrom.x + 30, sFrom.y - 20));
		AssertTrue(sTest.pcRamp->IsSelected());
		AssertSize(1, sTest.pcEditor->NumSelected());
		AssertInt(sNode.x + 30, sTest.pcRamp->GetNodePosition().x);
		AssertInt(sNode.y - 20, sTest.pcRamp->GetNodePosition().y);
		AssertInt(sNode.x + 30, sTest.pcRamp->GetPosition().x);
		AssertLog("Moved;");

		//Pressed and let go where it is, another is the selected one and nothing moved.
		sFrom = TestNodeEditorTitle(sTest.pcOut);
		DragTestNodeEditor(pWindow, sFrom, sFrom);
		AssertTrue(sTest.pcOut->IsSelected());
		AssertFalse(sTest.pcRamp->IsSelected());
		AssertLog("");

		//A box from the background over Noise and Ramp selects them, not Out; the selected two then move together.
		DragTestNodeEditor(pWindow, SInt32Vec2(5, 5), SInt32Vec2(300, 200));
		AssertTrue(sTest.pcNoise->IsSelected());
		AssertTrue(sTest.pcRamp->IsSelected());
		AssertFalse(sTest.pcOut->IsSelected());
		sNode = sTest.pcNoise->GetNodePosition();
		sFrom = TestNodeEditorTitle(sTest.pcRamp);
		DragTestNodeEditor(pWindow, sFrom, SInt32Vec2(sFrom.x, sFrom.y + 50));
		AssertInt(sNode.y + 50, sTest.pcNoise->GetNodePosition().y);
		AssertSize(2, sTest.pcEditor->NumSelected());
		AssertLog("Moved;");

		//The background, clicked, selects nothing; Delete asks the listener.
		DragTestNodeEditor(pWindow, SInt32Vec2(700, 350), SInt32Vec2(700, 350));
		AssertSize(0, sTest.pcEditor->NumSelected());
		memset(&sEvent, 0, sizeof(sEvent));
		sTest.pcEditor->DeleteAction(&sEvent);
		AssertLog("Delete;");
		AssertSize(3, sTest.pcEditor->NumNodes());

		pWindow->Paint();
		sTest.pcEditor->SetListener(NULL);
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		cListener.mszLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestNodeEditor(void)
{
	BeginTests();
	ObjectsInit();

	TestNodeEditorLayout();
	TestNodeEditorLinks();
	TestNodeEditorPointer();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

