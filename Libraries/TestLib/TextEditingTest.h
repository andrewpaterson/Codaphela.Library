#ifndef __TEXT_EDITING_TEST_H__
#define __TEXT_EDITING_TEST_H__
#include "BaseLib/TextEditing.h"
#include "BaseLib/ArrayInt.h"
#include "Assert.h"


#define TEST_TEXT_EDITING_PAGE_ROWS		3


//A text editing's view for tests (TestBaseLib's and TestWindowLib's): a row for each line of the text, a y for each row,
//a column of x for each character (tabs to tab stops), and a page of three rows.  It counts what it is told.
class CTestTextEditingView : public CTextEditingView
{
public:
	CTextDocument*	mpcDocument;
	int				miMarksChanged;
	int				miCaretShown;
	int				miTextEdited;
	CArrayInt		maiUndoChanges;
	size			muiLastIdAfter;
	int32			miColumnWidth;  //Each character's width in x.

	void	Init(CTextDocument* pcDocument)
	{
		mpcDocument = pcDocument;
		miColumnWidth = 1;
		miMarksChanged = 0;
		miCaretShown = 0;
		miTextEdited = 0;
		maiUndoChanges.Init();
		muiLastIdAfter = 0;
	}

	void	Kill(void) { maiUndoChanges.Kill(); }

	bool	HasRows(void) override { return true; }
	size	NumRows(void) override { return mpcDocument->NumLines(); }
	size	FindRow(size uiPosition, bool bRowEnd) override { return mpcDocument->FindLine(uiPosition); }
	size	GetRowStart(size uiRow) override { return mpcDocument->GetLineStart(uiRow); }

	size	GetRowEnd(size uiRow, bool* pbRowEnd) override
	{
		*pbRowEnd = false;
		return mpcDocument->GetNavigation()->FindLineEnd(mpcDocument->GetLineStart(uiRow));
	}

	size	FindRowAt(int32 y) override
	{
		if (y < 0)
		{
			return 0;
		}
		return ((size)y < NumRows()) ? (size)y : NumRows() - 1;
	}

	size	FindPositionInRow(size uiRow, int32 x, bool* pbRowEnd) override
	{
		*pbRowEnd = false;
		if (x < 0)
		{
			x = 0;
		}
		return mpcDocument->GetNavigation()->FindColumn(mpcDocument->GetLineStart(uiRow), (size)((x + miColumnWidth / 2) / miColumnWidth));
	}

	int32	GetPositionX(size uiPosition, bool bRowEnd) override
	{
		return (int32)mpcDocument->GetNavigation()->CountColumns(mpcDocument->GetNavigation()->FindLineStart(uiPosition), uiPosition) * miColumnWidth;
	}

	size	ScrollPage(size uiRow, bool bDown) override
	{
		if (bDown)
		{
			return (uiRow + TEST_TEXT_EDITING_PAGE_ROWS < NumRows()) ? uiRow + TEST_TEXT_EDITING_PAGE_ROWS : NumRows() - 1;
		}
		return (uiRow > TEST_TEXT_EDITING_PAGE_ROWS) ? uiRow - TEST_TEXT_EDITING_PAGE_ROWS : 0;
	}

	void	MarksChanged(void) override { miMarksChanged++; }
	void	ShowCaret(void) override { miCaretShown++; }
	void	TextEdited(void) override { miTextEdited++; }

	void	UndoChanged(STextUndoChange* psChange) override
	{
		maiUndoChanges.Add(psChange->eChange);
		muiLastIdAfter = psChange->uiIdAfter;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// pcEditing has a box selection whose rows' ranges are auiExpected's: a
// start and an end for each of uiNumRanges rows.
//
//////////////////////////////////////////////////////////////////////////
inline void AssertTextEditingBox(CTextEditing* pcEditing, size uiNumRanges, size* auiExpected)
{
	CArrayTextRange	asRanges;
	size			i;

	AssertTrue(pcEditing->IsBox());
	asRanges.Init();
	pcEditing->GetBoxRanges(&asRanges);
	AssertSize(uiNumRanges, asRanges.NumElements());
	for (i = 0; (i < uiNumRanges) && (i < asRanges.NumElements()); i++)
	{
		AssertSize(auiExpected[i * 2], asRanges.Get(i)->uiStart);
		AssertSize(auiExpected[i * 2 + 1], asRanges.Get(i)->uiEnd);
	}
	asRanges.Kill();
}


#endif // __TEXT_EDITING_TEST_H__

