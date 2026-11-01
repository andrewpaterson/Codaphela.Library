#include "BaseLib/PlainTextEditor.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPlainTextEditorInsertionAndTypeOver(void)
{
	CPlainTextEditor	cEditor;
	int					iChars;

	cEditor.Init("");
	cEditor.Printable('H');
	cEditor.Printable('e');
	cEditor.Printable('l');
	cEditor.Printable('l');
	cEditor.Printable('o');
	AssertString("Hello", cEditor.Text());
	AssertSize(5, cEditor.miEditPos);

	cEditor.Left();
	cEditor.Printable('x');
	AssertString("Hellxo", cEditor.Text());
	AssertSize(5, cEditor.miEditPos);

	cEditor.Backspace();
	AssertString("Hello", cEditor.Text());
	AssertSize(4, cEditor.miEditPos);

	cEditor.Home();
	cEditor.Paste("World ");
	AssertString("World Hello", cEditor.Text());
	AssertSize(6, cEditor.miEditPos);

	cEditor.Backspace();
	cEditor.Enter();
	AssertString("World\nHello", cEditor.Text());
	AssertSize(6, cEditor.miEditPos);

	cEditor.Right();
	cEditor.Right();
	cEditor.Right();
	cEditor.Right();
	cEditor.Right();
	cEditor.Enter();
	AssertString("World\nHello\n", cEditor.Text());
	AssertSize(12, cEditor.miEditPos);

	cEditor.Tab();
	cEditor.Printable('X');
	iChars = (int)cEditor.GetCharCountAlongLine(cEditor.miEditPos);
	AssertInt(5, iChars);
	iChars = (int)cEditor.GetCharCountAlongLine(cEditor.miEditPos-1);
	AssertInt(4, iChars);
	iChars = (int)cEditor.GetCharCountAlongLine(cEditor.miEditPos-2);
	AssertInt(0, iChars);

	cEditor.Left();
	cEditor.Up();
	AssertString("World\nHello\n\tX", cEditor.Text());
	AssertSize(10, cEditor.miEditPos);

	cEditor.Printable('O', false);
	cEditor.Printable('!', false);
	AssertString("World\nHellO!\n\tX", cEditor.Text());
	cEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPlainTextEditorSimpleNavigation(void)
{
	CPlainTextEditor	cEditor;
	int					iChars;

	cEditor.Init("");
	cEditor.Printable('H');
	cEditor.Printable('e');
	cEditor.Printable('l');
	cEditor.Printable('l');
	cEditor.Printable('o');
	AssertString("Hello", cEditor.Text());
	AssertSize(5, cEditor.miEditPos);

	cEditor.Left();
	cEditor.Printable('x');
	AssertString("Hellxo", cEditor.Text());
	AssertSize(5, cEditor.miEditPos);

	cEditor.Backspace();
	AssertString("Hello", cEditor.Text());
	AssertSize(4, cEditor.miEditPos);

	cEditor.Home();
	cEditor.Paste("World ");
	AssertString("World Hello", cEditor.Text());
	AssertSize(6, cEditor.miEditPos);

	cEditor.Backspace();
	cEditor.Enter();
	AssertString("World\nHello", cEditor.Text());
	AssertSize(6, cEditor.miEditPos);

	cEditor.Right();
	cEditor.Right();
	cEditor.Right();
	cEditor.Right();
	cEditor.Right();
	cEditor.Enter();
	AssertString("World\nHello\n", cEditor.Text());
	AssertSize(12, cEditor.miEditPos);

	cEditor.Tab();
	cEditor.Printable('X');
	iChars = (int)cEditor.GetCharCountAlongLine(cEditor.miEditPos);
	AssertInt(5, iChars);
	iChars = (int)cEditor.GetCharCountAlongLine(cEditor.miEditPos-1);
	AssertInt(4, iChars);
	iChars = (int)cEditor.GetCharCountAlongLine(cEditor.miEditPos-2);
	AssertInt(0, iChars);

	cEditor.Left();
	cEditor.Up();
	AssertString("World\nHello\n\tX", cEditor.Text());
	AssertSize(10, cEditor.miEditPos);

	cEditor.Printable('O', false);
	cEditor.Printable('!', false);
	AssertString("World\nHellO!\n\tX", cEditor.Text());
	cEditor.Kill();

	cEditor.Init("\tX");
	cEditor.Home();
	cEditor.Printable('a', false);
	AssertString("a\tX", cEditor.Text());
	cEditor.Printable('b', false);
	AssertString("ab\tX", cEditor.Text());
	cEditor.Printable('c', false);
	AssertString("abc\tX", cEditor.Text());
	cEditor.Printable('d', false);
	AssertString("abcdX", cEditor.Text());
	cEditor.Printable('Y', false);
	AssertString("abcdY", cEditor.Text());
	cEditor.Left();
	cEditor.Delete();
	AssertString("abcd", cEditor.Text());
	cEditor.Delete();
	AssertString("abcd", cEditor.Text());

	cEditor.Home();
	AssertSize(0, cEditor.miEditPos);
	cEditor.End();
	AssertSize(4, cEditor.miEditPos);
	cEditor.Enter();
	cEditor.Paste("efgh");
	AssertString("abcd\nefgh", cEditor.Text());
	cEditor.Home();
	AssertSize(5, cEditor.miEditPos);
	cEditor.Up();
	AssertSize(0, cEditor.miEditPos);
	cEditor.End();
	AssertSize(4, cEditor.miEditPos);
	cEditor.Down();
	AssertSize(9, cEditor.miEditPos);
	cEditor.End();
	AssertSize(9, cEditor.miEditPos);
	cEditor.Left();
	cEditor.End();
	AssertSize(9, cEditor.miEditPos);
	cEditor.Home();
	cEditor.Up();
	cEditor.Right();
	cEditor.Down();
	AssertSize(6, cEditor.miEditPos);
	cEditor.Down();
	AssertSize(9, cEditor.miEditPos);

	cEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPlainTextEditorUpDownNavigation(void)
{
	CPlainTextEditor	cEditor;

	cEditor.Init("\
zzzzzzz\n\
\n\
zzz\n\
zzzzzzz\n\
zzz\n\
zz\n\
zzzz\n\
\n\
zzzzzzz");

	cEditor.Right();
	cEditor.Right();
	cEditor.Right();
	cEditor.Right();
	AssertSize(4, cEditor.miEditPos);

	cEditor.Down();
	AssertSize(8, cEditor.miEditPos);
	cEditor.Down();
	AssertSize(12, cEditor.miEditPos);
	cEditor.Down();
	AssertSize(17, cEditor.miEditPos);

	cEditor.Up();
	AssertSize(12, cEditor.miEditPos);
	cEditor.Up();
	AssertSize(8, cEditor.miEditPos);
	cEditor.Up();
	AssertSize(4, cEditor.miEditPos);
	cEditor.Down();
	cEditor.Down();
	cEditor.Left();
	cEditor.Down();
	AssertSize(15, cEditor.miEditPos);

	cEditor.DocumentHome();
	cEditor.Down();
	cEditor.Down();
	AssertSize(9, cEditor.miEditPos);

	cEditor.DocumentEnd();
	cEditor.PageUp();
	AssertSize(7, cEditor.miEditPos);

	cEditor.Left();
	cEditor.Left();
	cEditor.Left();
	cEditor.SetPageLength(3);
	cEditor.PageDown();
	AssertSize(17, cEditor.miEditPos);
	cEditor.PageDown();
	AssertSize(32, cEditor.miEditPos);
	cEditor.PageUp();
	AssertSize(17, cEditor.miEditPos);
	cEditor.PageDown();
	cEditor.PageDown();
	AssertSize(38, cEditor.miEditPos);
	cEditor.PageUp();
	AssertSize(27, cEditor.miEditPos);
	cEditor.PageUp();
	AssertSize(12, cEditor.miEditPos);
	cEditor.PageUp();
	AssertSize(4, cEditor.miEditPos);

	cEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPlainTextEditorWordNavigation(void)
{
	CPlainTextEditor	cEditor;

	cEditor.Init("\t\tHello World()\n\t\tEndOfLine;Nah!");

	cEditor.WordRight();
	AssertSize(2, cEditor.miEditPos);
	cEditor.WordRight();
	AssertSize(7, cEditor.miEditPos);
	cEditor.WordRight();
	AssertSize(8, cEditor.miEditPos);	
	cEditor.WordRight();
	AssertSize(13, cEditor.miEditPos);
	cEditor.WordRight();
	AssertSize(14, cEditor.miEditPos);
	cEditor.WordRight();
	AssertSize(15, cEditor.miEditPos);
	cEditor.WordRight();
	AssertSize(18, cEditor.miEditPos);
	cEditor.WordRight();
	AssertSize(27, cEditor.miEditPos);
	cEditor.WordRight();
	AssertSize(28, cEditor.miEditPos);
	cEditor.WordRight();
	AssertSize(31, cEditor.miEditPos);
	cEditor.WordRight();
	AssertSize(32, cEditor.miEditPos);
	cEditor.WordRight();
	AssertSize(32, cEditor.miEditPos);

	cEditor.WordLeft();
	AssertSize(31, cEditor.miEditPos);
	cEditor.WordLeft();
	AssertSize(28, cEditor.miEditPos);
	cEditor.WordLeft();
	AssertSize(27, cEditor.miEditPos);	
	cEditor.WordLeft();
	AssertSize(18, cEditor.miEditPos);
	cEditor.WordLeft();
	AssertSize(15, cEditor.miEditPos);
	cEditor.WordLeft();
	AssertSize(14, cEditor.miEditPos);
	cEditor.WordLeft();
	AssertSize(13, cEditor.miEditPos);
	cEditor.WordLeft();
	AssertSize(8, cEditor.miEditPos);
	cEditor.WordLeft();
	AssertSize(7, cEditor.miEditPos);
	cEditor.WordLeft();
	AssertSize(2, cEditor.miEditPos);
	cEditor.WordLeft();
	AssertSize(0, cEditor.miEditPos);
	cEditor.WordLeft();
	AssertSize(0, cEditor.miEditPos);

	cEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPlainTextEditorSelection(void)
{
	CPlainTextEditor	cEditor;
	CChars				szSelection;

	cEditor.Init("ABCDE\nFGHIJ\nKLMNO\nPQRST\nUVWXY");

	cEditor.SelectRight();
	AssertSize(1, cEditor.miEditPos);
	AssertSize(0, cEditor.miSelectionAnchor);
	AssertSize(1, cEditor.miSelectionFloating);

	cEditor.SelectRight();
	AssertSize(2, cEditor.miEditPos);
	AssertSize(0, cEditor.miSelectionAnchor);
	AssertSize(2, cEditor.miSelectionFloating);

	cEditor.Right();
	AssertSize(3, cEditor.miEditPos);
	AssertSize(-1, cEditor.miSelectionAnchor);
	AssertSize(-1, cEditor.miSelectionFloating);

	cEditor.SelectDown();
	cEditor.SelectDown();
	AssertSize(15, cEditor.miEditPos);
	AssertSize(3, cEditor.miSelectionAnchor);
	AssertSize(15, cEditor.miSelectionFloating);
	
	szSelection.Init();
	cEditor.Copy(&szSelection);
	AssertString("DE\nFGHIJ\nKLM", szSelection.Text());
	szSelection.Kill();

	cEditor.DocumentHome();
	cEditor.SelectDocumentEnd();
	AssertSize(0, cEditor.miSelectionAnchor);
	AssertSize(29, cEditor.miSelectionFloating);

	szSelection.Init();
	cEditor.Copy(&szSelection);
	AssertString("ABCDE\nFGHIJ\nKLMNO\nPQRST\nUVWXY", szSelection.Text());
	szSelection.Kill();

	cEditor.SetPosition(8);
	cEditor.SelectWholeWord();
	szSelection.Init();
	cEditor.Copy(&szSelection);
	AssertString("FGHIJ", szSelection.Text());
	szSelection.Kill();
	AssertSize(6, cEditor.miSelectionAnchor);
	AssertSize(11, cEditor.miSelectionFloating);

	cEditor.SetPageLength(2);
	cEditor.SelectPageDown();
	szSelection.Init();
	cEditor.Copy(&szSelection);
	AssertString("FGHIJ\nKLMNO\nPQRST", szSelection.Text());
	szSelection.Kill();
	AssertSize(6, cEditor.miSelectionAnchor);
	AssertSize(23, cEditor.miSelectionFloating);

	cEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPlainTextEditorCopyPaste(void)
{
	CPlainTextEditor	cEditor;
	CChars				szSelection;
	CChars				szline;

	cEditor.Init("ABCDE\nFGHIJ\nKLMNO\nPQRST\nUVWXY");
	cEditor.SetPosition(8);
	szSelection.Init();
	cEditor.CutLine(&szSelection);
	AssertString("FGHIJ\n", szSelection.Text());
	szSelection.Kill();
	AssertSize(8, cEditor.miEditPos);
	AssertSize(2, cEditor.GetCharCountAlongLine(cEditor.miEditPos));
	szline.Init();
	AssertString("MNO", cEditor.Line(&szline));
	szline.Kill();

	cEditor.Down();
	szline.Init();
	AssertString("RST", cEditor.Line(&szline));
	szline.Kill();

	cEditor.Down();
	szline.Init();
	AssertString("WXY", cEditor.Line(&szline));
	szline.Kill();

	szSelection.Init();
	cEditor.CutLine(&szSelection);
	AssertString("UVWXY", szSelection.Text());
	szSelection.Kill();
	AssertSize(18, cEditor.miEditPos);
	AssertSize(0, cEditor.GetCharCountAlongLine(cEditor.miEditPos));
	szline.Init();
	AssertString("", cEditor.Line(&szline));
	szline.Kill();

	cEditor.DocumentHome();
	cEditor.Right();

	szline.Init();
	AssertString("BCDE", cEditor.Line(&szline));
	szline.Kill();

	szSelection.Init();
	cEditor.CutLine(&szSelection);
	AssertString("ABCDE\n", szSelection.Text());
	szSelection.Kill();
	AssertSize(1, cEditor.miEditPos);
	AssertSize(1, cEditor.GetCharCountAlongLine(cEditor.miEditPos));

	AssertString("KLMNO\nPQRST\n", cEditor.Text());
	cEditor.Kill();

	cEditor.Init("ABCDE\nFGHIJ\nKLMNO\nPQRST\nUVWXY");
	cEditor.SetPosition(8);
	cEditor.SelectDown();
	cEditor.SelectDown();
	cEditor.SelectRight();
	cEditor.DeleteLine();

	AssertString("ABCDE\nUVWXY", cEditor.Text());
	AssertSize(9, cEditor.miEditPos);
	AssertSize(3, cEditor.GetCharCountAlongLine(cEditor.miEditPos));

	cEditor.SetPosition(2);
	cEditor.SelectDown();
	cEditor.DeleteLine();
	AssertString("", cEditor.Text());

	cEditor.Kill();

	cEditor.Init("ABCDE\nFGHIJ\nKLMNO");
	cEditor.Right();
	cEditor.DeleteWordRemainingRight();
	AssertString("A\nFGHIJ\nKLMNO", cEditor.Text());
	cEditor.DeleteWordRemainingRight();
	AssertString("AFGHIJ\nKLMNO", cEditor.Text());
	cEditor.DeleteWordRemainingRight();
	AssertString("A\nKLMNO", cEditor.Text());
	cEditor.DeleteWordRemainingRight();
	AssertString("AKLMNO", cEditor.Text());
	cEditor.DeleteWordRemainingRight();
	AssertString("A", cEditor.Text());

	cEditor.Kill();

	cEditor.Init("ABCDE\nFGHIJ\nKLMNO");
	
	cEditor.SetPosition(8);
	cEditor.Duplicate();
	AssertString("ABCDE\nFGHIJ\nFGHIJ\nKLMNO", cEditor.Text());
	AssertSize(8, cEditor.miEditPos);

	cEditor.SetPosition(1);
	cEditor.SelectRight();
	cEditor.SelectRight();
	cEditor.SelectRight();
	cEditor.Duplicate();
	AssertString("ABCDBCDE\nFGHIJ\nFGHIJ\nKLMNO", cEditor.Text());
	AssertSize(4, cEditor.miEditPos);

	cEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPlainTextEditorSelectionOverwrite(void)
{
	CPlainTextEditor	cEditor;
	CChars				szTemp;

	cEditor.Init("The Quick Brown Fox Blah, Blah, Blah.");
	cEditor.WordRight();
	cEditor.WordRight();
	cEditor.SelectWordRight();

	szTemp.Init();
	cEditor.Cut(&szTemp);
	cEditor.Delete();
	cEditor.WordRight();
	cEditor.WordRight();
	cEditor.SelectWordRight();
	cEditor.Paste(szTemp.Text());
	szTemp.Kill();
	AssertString("The Brown Quick Blah, Blah, Blah.", cEditor.Text());

	cEditor.Right();
	cEditor.Right();
	cEditor.SelectWholeWord();
	cEditor.Printable('X');
	AssertString("The Brown Quick X, Blah, Blah.", cEditor.Text());

	cEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPlainTextEditorDeleteLine(void)
{
	CPlainTextEditor	cEditor;

	cEditor.Init("A scary tale.");
	cEditor.End();
	cEditor.Enter();
	cEditor.Paste("Fairy folk and evil doers.");
	cEditor.DeleteLine();
	cEditor.Paste("Black Jack and Henry.");
	AssertString("A scary tale.\nBlack Jack and Henry.", cEditor.Text());

	AssertSize(35, cEditor.miEditPos);
	AssertSize(21, cEditor.GetCharCountAlongLine(cEditor.miEditPos));
	cEditor.Up();
	AssertSize(13, cEditor.miEditPos);
	AssertSize(21, cEditor.miUpDownCharCount);

	cEditor.DeleteLine();
	AssertString("Black Jack and Henry.", cEditor.Text());
	AssertSize(13, cEditor.miEditPos);
	cEditor.Home();
	cEditor.Paste("A terrifying tale:");
	cEditor.Enter();
	AssertString("A terrifying tale:\nBlack Jack and Henry.", cEditor.Text());
	AssertSize(19, cEditor.miEditPos);

	cEditor.DeleteLine();
	AssertString("A terrifying tale:\n", cEditor.Text());
	AssertSize(19, cEditor.miEditPos);
	cEditor.DeleteLine();
	AssertString("A terrifying tale:\n", cEditor.Text());
	AssertSize(19, cEditor.miEditPos);

	cEditor.Left();
	cEditor.DeleteLine();
	AssertString("", cEditor.Text());
	AssertSize(0, cEditor.miEditPos);

	cEditor.DeleteLine();
	AssertString("", cEditor.Text());
	AssertSize(0, cEditor.miEditPos);

	cEditor.Paste("Once upon a time.");
	cEditor.Enter();
	cEditor.Paste("There was an Astrogat.");
	cEditor.DocumentHome();
	AssertSize(0, cEditor.miEditPos);

	cEditor.DeleteLine();
	AssertString("There was an Astrogat.", cEditor.Text());
	cEditor.DeleteLine();
	AssertString("", cEditor.Text());
	AssertSize(0, cEditor.miEditPos);

	cEditor.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPlainTextEditorLeftRightDelete(void)
{
	CPlainTextEditor	cEditor;

	cEditor.Init();
	cEditor.Printable('H');
	cEditor.Left();
	cEditor.Delete();

	AssertString("", cEditor.Text());
	AssertSize(0, cEditor.miEditPos);

	cEditor.Left();
	AssertSize(0, cEditor.miEditPos);

	cEditor.Delete();
	AssertSize(0, cEditor.miEditPos);

	cEditor.Right();
	AssertSize(0, cEditor.miEditPos);


	cEditor.Printable('H');
	AssertString("H", cEditor.Text());
	cEditor.Left();
	AssertSize(0, cEditor.miEditPos);
	cEditor.Backspace();
	AssertSize(0, cEditor.miEditPos);
	cEditor.Right();
	AssertSize(1, cEditor.miEditPos);
	cEditor.Delete();
	AssertSize(1, cEditor.miEditPos);
	AssertString("H", cEditor.Text());

	cEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPlainTextEditor(void)
{
	BeginTests();

	TestPlainTextEditorInsertionAndTypeOver();
	TestPlainTextEditorSimpleNavigation();
	TestPlainTextEditorUpDownNavigation();
	TestPlainTextEditorWordNavigation();
	TestPlainTextEditorSelection();
	TestPlainTextEditorCopyPaste();
	TestPlainTextEditorSelectionOverwrite();
	TestPlainTextEditorDeleteLine();
	TestPlainTextEditorLeftRightDelete();

	TestStatistics();
}
