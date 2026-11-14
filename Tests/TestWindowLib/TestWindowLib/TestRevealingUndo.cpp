#include "BaseLib/GlobalMemory.h"
#include "WindowLib/RevealingUndo.h"
#include "TestLib/Assert.h"


//An editor with changes at places: the next undo's change is at masPlaces' last, the editor is at miHere.
class CTestUndoHost : public CRevealingUndoHost
{
public:
	int		miHere;
	int		miChange;  //Where the next undo's change is, -1 for none.
	int		miUndone;
	CChars	mszLog;

	void Init(void) { miHere = 0; miChange = 0; miUndone = 0; mszLog.Init(); }
	void Kill(void) { mszLog.Kill(); }

	bool IsUndoShown(bool bRedo) override { return miChange == miHere; }
	bool ShowUndo(bool bRedo) override { if (miChange < 0) { return false; } miHere = miChange; mszLog.Append("show;"); return true; }
	bool DoUndo(bool bRedo) override { if (miChange < 0) { return false; } miUndone++; mszLog.Append(bRedo ? "redo;" : "undo;"); return true; }
	void AppendUndoWhere(bool bRedo, CChars* pszDest) override { pszDest->Append("in place "); pszDest->Append(miChange); }
};


//////////////////////////////////////////////////////////////////////////
//
// A change in view is undone at once; one out of view is shown by the
// first press, said, and undone by the second, as it's in view then;
// moving away in between shows it again.
//
//////////////////////////////////////////////////////////////////////////
void TestRevealingUndoPresses(void)
{
	CTestUndoHost	cHost;
	CRevealingUndo	cUndo;
	CChars			szMessage;

	cHost.Init();
	cUndo.Init(&cHost);
	szMessage.Init();

	AssertTrue(cUndo.Press(false, "Ctrl+Z", &szMessage));
	AssertString("undo;", cHost.mszLog.Text());
	AssertString("", szMessage.Text());

	cHost.mszLog.Clear();
	cHost.miChange = 2;
	AssertTrue(cUndo.Press(false, "Ctrl+Z", &szMessage));
	AssertString("show;", cHost.mszLog.Text());
	AssertString("Ctrl+Z again to undo the change in place 2.", szMessage.Text());
	AssertTrue(cUndo.Press(false, "Ctrl+Z", &szMessage));
	AssertString("show;undo;", cHost.mszLog.Text());
	AssertString("", szMessage.Text());

	//Moving away between the presses shows it again; so does redo after an undo's first press.
	cHost.mszLog.Clear();
	cHost.miChange = 3;
	AssertTrue(cUndo.Press(false, "Ctrl+Z", &szMessage));
	cHost.miHere = 1;
	AssertTrue(cUndo.Press(false, "Ctrl+Z", &szMessage));
	AssertString("show;show;", cHost.mszLog.Text());
	AssertTrue(cUndo.Press(true, "Ctrl+Shift+Z", &szMessage));
	AssertString("show;show;redo;", cHost.mszLog.Text());
	cHost.miChange = 4;
	AssertTrue(cUndo.Press(true, "Ctrl+Shift+Z", &szMessage));
	AssertString("Ctrl+Shift+Z again to redo the change in place 4.", szMessage.Text());

	//Nothing to undo.
	cHost.miChange = -1;
	cHost.mszLog.Clear();
	AssertFalse(cUndo.Press(false, "Ctrl+Z", &szMessage));
	AssertString("", cHost.mszLog.Text());

	szMessage.Kill();
	cUndo.Kill();
	cHost.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRevealingUndo(void)
{
	BeginTests();

	TestRevealingUndoPresses();

	TestStatistics();
}

