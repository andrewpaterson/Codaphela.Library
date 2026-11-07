/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/FileUtil.h"
#include "BaseLib/StringHelper.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "FillContainer.h"
#include "RoundedBoxCanvasDraw.h"
#include "IconComponent.h"
#include "FileChooser.h"


#define FILE_CHOOSER_WIDTH			620
#define FILE_CHOOSER_HEIGHT			440
#define FILE_CHOOSER_MINIMUM_WIDTH	420  //It can be resized, no smaller than this.
#define FILE_CHOOSER_MINIMUM_HEIGHT	300
#define FILE_CHOOSER_ROW_HEIGHT		24
#define FILE_CHOOSER_ICON_WIDTH		30  //The column of the icons, a name starts after it.
#define FILE_CHOOSER_GAP			8  //Between the rows of the dialog, and between the buttons along the top.
#define FILE_CHOOSER_LABEL_GAP		12  //Between a label and what it labels.
#define NEW_FOLDER_DIALOG_WIDTH		360


//////////////////////////////////////////////////////////////////////////
//
// A whole path without a trailing separator, unless it's a root.  A drive
// alone ("D:") is its root.
//
//////////////////////////////////////////////////////////////////////////
void NormaliseDirectory(CChars* pszDirectory)
{
	CFileUtil	cFileUtil;

	if (pszDirectory->EndsWith(':'))
	{
		pszDirectory->Append(FILE_SEPARATOR);
	}
	cFileUtil.FullPath(pszDirectory);
	if (pszDirectory->FindFromEnd(FILE_SEPARATOR[0]) == ARRAY_ELEMENT_NOT_FOUND)
	{
		pszDirectory->Append(FILE_SEPARATOR);
	}
	else if ((pszDirectory->Count(FILE_SEPARATOR[0]) > 1) && pszDirectory->EndsWith(FILE_SEPARATOR[0]))
	{
		pszDirectory->RemoveEnd(pszDirectory->Length() - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Starts in the folder of szFilename with the file selected and its name
// in File Name, or in the current folder if that folder doesn't exist.
// asTypes are Files of Type's choices.  The icons are shown beside the
// folders and files.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::Init(Ptr<CWindow> pOwner, SDialogLook* psLook, const char* szTitle, bool bSave, const char* szFilename, SFileChooserType* asTypes, size uiNumTypes, Ptr<CImageCel> pFolderIcon, Ptr<CImageCel> pFileIcon)
{
	CFileUtil		cFileUtil;
	Ptr<CComponent>	pBody;
	CChars			szPath;
	CChars			szName;
	CChars			szDirectory;
	size			i;

	PreInit();

	mbSave = bSave;
	mbNewFolderPending = false;
	mbReplacePending = false;
	masTypes = asTypes;
	muiNumTypes = uiNumTypes;
	mpFolderIcon = pFolderIcon;
	mpFileIcon = pFileIcon;
	mszDirectory.Init();
	maszLookIn.Init();
	maszRows.Init();
	miNumFolders = 0;
	mszChosen.Init();

	mpDialog = OMalloc<CDialog>(pOwner, psLook, szTitle);
	mpDialog->SetOKCaption(bSave ? "Save" : "Open");
	mpDialog->SetResizable(FILE_CHOOSER_MINIMUM_WIDTH, FILE_CHOOSER_MINIMUM_HEIGHT);
	mpDialog->SetApprover(this, &CFileChooser::Approve);
	mpDialog->SetTicker(this, &CFileChooser::DialogTick);
	pBody = CreateBody();
	mpDialog->SetBody(pBody, mpFileName);

	for (i = 0; i < muiNumTypes; i++)
	{
		mpFileType->AddItem(masTypes[i].szDescription);
	}
	mpFileType->Select(0);
	mpFileType->GetSelectionEvent()->Add(this, &CFileChooser::FileTypeChosen);
	mpLookIn->GetSelectionEvent()->Add(this, &CFileChooser::LookInChosen);
	mpList->GetSelectionEvent()->Add(this, &CFileChooser::SelectionChanged);
	mpList->GetActivationEvent()->Add(this, &CFileChooser::ItemActivated);

	szPath.Init(szFilename);
	if (!szPath.Empty())
	{
		cFileUtil.FullPath(&szPath);
	}
	szName.Init();
	szDirectory.Init();
	cFileUtil.SplitPath(szPath.Text(), &szName, &szDirectory);
	if (szDirectory.Empty() || !Open(szDirectory.Text(), szPath.Text()))
	{
		szDirectory.Kill();
		szDirectory.Init();
		cFileUtil.CurrentDirectory(&szDirectory);
		Open(szDirectory.Text(), NULL);
	}
	mpFileName->SetText(szName.Text());
	mpFileName->SelectAll();
	szDirectory.Kill();
	szName.Kill();
	szPath.Kill();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::Class(void)
{
	M_Pointer(mpDialog);
	M_Pointer(mpNewFolderDialog);
	M_Pointer(mpReplaceDialog);
	M_Pointer(mpLookIn);
	M_Pointer(mpUp);
	M_Pointer(mpHome);
	M_Pointer(mpNewFolder);
	M_Pointer(mpScrollPane);
	M_Pointer(mpList);
	M_Pointer(mpFileName);
	M_Pointer(mpFileType);
	M_Pointer(mpFolderIcon);
	M_Pointer(mpFileIcon);
	U_String(mszDirectory);
	U_Unknown(CArrayChars, maszLookIn);
	U_Unknown(CArrayChars, maszRows);
	U_Int32(miNumFolders);
	U_String(mszChosen);
	U_Bool(mbSave);
	U_Bool(mbNewFolderPending);
	U_Bool(mbReplacePending);
	U_Pointer(masTypes);
	U_Data(size, muiNumTypes);
}


//////////////////////////////////////////////////////////////////////////
//
// A chooser isn't saved.
//
//////////////////////////////////////////////////////////////////////////
bool CFileChooser::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileChooser::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::Free(void)
{
	//An object's memory is freed without its destructor running so the chooser takes itself off the widgets still
	//here.  The dialog and widgets are freed with the chooser's pointers.
	if (mpDialog.IsNotNull())
	{
		mpDialog->ClearApprover();
		mpDialog->ClearTicker();
	}
	if (mpFileType.IsNotNull())
	{
		mpFileType->GetSelectionEvent()->Remove(this);
	}
	if (mpLookIn.IsNotNull())
	{
		mpLookIn->GetSelectionEvent()->Remove(this);
	}
	if (mpList.IsNotNull())
	{
		mpList->GetSelectionEvent()->Remove(this);
		mpList->GetActivationEvent()->Remove(this);
	}
	RemoveFromButton(mpUp);
	RemoveFromButton(mpHome);
	RemoveFromButton(mpNewFolder);

	mszChosen.Kill();
	maszRows.Kill();
	maszLookIn.Kill();
	mszDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Look In and its buttons along the top, the folder's contents under
// them, then File Name and Files of Type, their labels lined up.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CFileChooser::CreateBody(void)
{
	Ptr<CWindow>			pWindow;
	SDialogLook*			psLook;
	Ptr<CGridContainer>		pBody;
	Ptr<CGridContainer>		pTop;
	Ptr<CFillContainer>		pGap;
	Ptr<CLabel>				pLabel;
	SInsets					sInsets;
	int32					iLabelWidth;
	int						iRow;

	pWindow = mpDialog->GetWindow();
	psLook = mpDialog->GetLook();
	pBody = OMalloc<CGridContainer>(pWindow);

	//Every label is as wide as the widest.
	pLabel = OMalloc<CLabel>(pWindow, psLook->pcTheme->GetFont(), "Files of Type:");
	iLabelWidth = pLabel->GetRequiredSize().x + FILE_CHOOSER_LABEL_GAP;

	//Look In and its buttons are a row of their own, so the buttons don't widen the labels' column.
	iRow = 0;
	pTop = OMalloc<CGridContainer>(pWindow);
	pTop->SetAlignment(CSH_Stretch, CSV_Top);
	pTop->AddComponent(CreateLabel(mbSave ? "Save In:" : "Look In:", iLabelWidth), 0, 0);
	mpLookIn = OMalloc<CComboBox>(pWindow, psLook->pcTheme);
	mpLookIn->SetAlignment(CSH_Stretch, CSV_Center);
	pTop->AddComponent(mpLookIn, 1, 0, 1, 1, 1, 0);
	mpUp = CreateButton("Up");
	mpHome = CreateButton("Home");
	mpNewFolder = CreateButton("New Folder");
	mpUp->GetClickEvent()->Add(this, &CFileChooser::GoUp);
	mpHome->GetClickEvent()->Add(this, &CFileChooser::GoHome);
	mpNewFolder->GetClickEvent()->Add(this, &CFileChooser::NewFolderClicked);
	AddButton(pTop, mpUp, 2, FILE_CHOOSER_GAP);
	AddButton(pTop, mpHome, 4, FILE_CHOOSER_GAP / 2);
	AddButton(pTop, mpNewFolder, 6, FILE_CHOOSER_GAP / 2);
	//A grid in a grid is the size it's given, not the size its parts need.
	pTop->SetDesiredSize(-1, (mpLookIn->GetDesiredSize().y > mpUp->GetDesiredSize().y) ? mpLookIn->GetDesiredSize().y : mpUp->GetDesiredSize().y);
	pBody->AddComponent(pTop, 0, iRow, 3, 1, 1, 0);
	iRow++;

	pGap = OMalloc<CFillContainer>(pWindow);
	pGap->SetDesiredSize(FILE_CHOOSER_GAP, FILE_CHOOSER_GAP);
	pBody->AddComponent(pGap, 0, iRow);
	iRow++;

	sInsets.Init(4, 0);
	mpList = OMalloc<CListContainer>(pWindow, psLook->pcTheme);
	mpList->SetHighlightDraws(OMalloc<CRoundedBoxCanvasDraw>(psLook->pcTheme->GetColour(TC_ListSelected), 4, &sInsets), OMalloc<CRoundedBoxCanvasDraw>(psLook->pcTheme->GetColour(TC_ListHover), 4, &sInsets));
	mpList->SetAlignment(CSH_Stretch, CSV_Top);
	mpList->SetDoubleClickActivates(true);
	mpScrollPane = OMalloc<CScrollPane>(pWindow, psLook->pcTheme);
	mpScrollPane->SetContent(mpList);
	mpScrollPane->SetAlignment(CSH_Stretch, CSV_Stretch);
	pBody->AddComponent(mpScrollPane, 0, iRow, 3, 1, 1, 1);
	iRow++;

	pGap = OMalloc<CFillContainer>(pWindow);
	pGap->SetDesiredSize(FILE_CHOOSER_GAP, FILE_CHOOSER_GAP);
	pBody->AddComponent(pGap, 0, iRow);
	iRow++;

	pBody->AddComponent(CreateLabel("File Name:", iLabelWidth), 0, iRow);
	mpFileName = OMalloc<CTextField>(pWindow, psLook->pcTheme, "");
	mpFileName->SetColumns(10);
	mpFileName->SetAlignment(CSH_Stretch, CSV_Center);
	pBody->AddComponent(mpFileName, 1, iRow, 2, 1, 1, 0);
	iRow++;

	pGap = OMalloc<CFillContainer>(pWindow);
	pGap->SetDesiredSize(FILE_CHOOSER_GAP, FILE_CHOOSER_GAP / 2);
	pBody->AddComponent(pGap, 0, iRow);
	iRow++;

	pBody->AddComponent(CreateLabel("Files of Type:", iLabelWidth), 0, iRow);
	mpFileType = OMalloc<CComboBox>(pWindow, psLook->pcTheme);
	mpFileType->SetAlignment(CSH_Stretch, CSV_Center);
	pBody->AddComponent(mpFileType, 1, iRow, 2, 1, 1, 0);
	return pBody;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CLabel> CFileChooser::CreateLabel(const char* szText, int32 iWidth)
{
	Ptr<CLabel>		pLabel;

	pLabel = OMalloc<CLabel>(mpDialog->GetWindow(), mpDialog->GetLook()->pcTheme->GetFont(), szText);
	pLabel->SetDesiredSize(iWidth, -1);
	pLabel->SetAlignment(CSH_Left, CSV_Center);
	return pLabel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButton> CFileChooser::CreateButton(const char* szCaption)
{
	Ptr<CButton>	pButton;

	pButton = OMalloc<CButton>(mpDialog->GetWindow(), mpDialog->GetLook()->pcTheme, szCaption);
	pButton->SetAlignment(CSH_Center, CSV_Center);
	return pButton;
}


//////////////////////////////////////////////////////////////////////////
//
// The button in iColumn of the row, after a gap in the column before.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::AddButton(Ptr<CGridContainer> pRow, Ptr<CButton> pButton, int iColumn, int32 iGap)
{
	Ptr<CFillContainer>		pGap;

	pGap = OMalloc<CFillContainer>(mpDialog->GetWindow());
	pGap->SetDesiredSize(iGap, 1);
	pRow->AddComponent(pGap, iColumn - 1, 0);
	pRow->AddComponent(pButton, iColumn, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the folder, the row of szSelect (a whole path) selected if it's
// in it, otherwise none.  Returns false, showing what was shown, if the
// folder doesn't exist.
//
//////////////////////////////////////////////////////////////////////////
bool CFileChooser::Open(const char* szDirectory, const char* szSelect)
{
	CFileUtil	cFileUtil;
	CChars		szPath;
	CChars		szSelectPath;

	//Copied first, either can be one of Look In's or the rows' paths.
	szPath.Init(szDirectory);
	szSelectPath.Init();
	if (szSelect)
	{
		szSelectPath.Append(szSelect);
	}
	NormaliseDirectory(&szPath);
	if (!cFileUtil.IsDirectory(szPath.Text()))
	{
		szSelectPath.Kill();
		szPath.Kill();
		return false;
	}

	mszDirectory.Set(szPath);
	FillLookIn();
	FillList(szSelectPath.Empty() ? NULL : szSelectPath.Text());
	mpScrollPane->SetScrollPosition(0, 0);
	szSelectPath.Kill();
	szPath.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The root first and the folder shown last, as JFileChooser's combo box
// has them.  Each is its whole path rather than its name indented, which
// reads oddly with the list closed.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::FillLookIn(void)
{
	CFileUtil		cFileUtil;
	CArrayChars		aszUp;  //The folder shown first, the root last.
	CChars			szPath;
	size			i;

	aszUp.Init();
	szPath.Init(mszDirectory);
	for (;;)
	{
		aszUp.Add(szPath.Text());
		if (cFileUtil.IsRootDirectory(szPath.Text()))
		{
			break;
		}
		cFileUtil.RemoveLastFromPath(&szPath);
		NormaliseDirectory(&szPath);
	}
	szPath.Kill();

	maszLookIn.ReInit();
	mpLookIn->RemoveAllItems();
	for (i = aszUp.NumElements(); i > 0; i--)
	{
		maszLookIn.Add(aszUp.GetText(i - 1));
		mpLookIn->AddItem(aszUp.GetText(i - 1));
	}
	mpLookIn->Select((int32)maszLookIn.NumElements() - 1);
	aszUp.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The folder's folders then its files of the type chosen, hidden ones
// left out.  The row of szSelect is selected if it's shown.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::FillList(const char* szSelect)
{
	CFileUtil		cFileUtil;
	CArrayChars		aszFolders;
	CArrayChars		aszFiles;
	size			i;
	int32			iSelect;

	//The chooser doesn't hear the rows it takes away and puts back.
	mpList->GetSelectionEvent()->Remove(this);
	mpList->RemoveAllComponents();
	maszRows.ReInit();

	aszFolders.Init();
	aszFiles.Init();
	cFileUtil.FindAllDirectories(mszDirectory.Text(), &aszFolders, false);
	cFileUtil.FindAllFiles(mszDirectory.Text(), &aszFiles, false, false);

	iSelect = -1;
	for (i = 0; i < aszFolders.NumElements(); i++)
	{
		if (szSelect && (StringCompare(aszFolders.GetText(i), szSelect) == 0))
		{
			iSelect = mpList->NumRows();
		}
		AddRow(aszFolders.GetText(i), true);
	}
	miNumFolders = mpList->NumRows();
	for (i = 0; i < aszFiles.NumElements(); i++)
	{
		if (IsShownFile(aszFiles.GetText(i)))
		{
			if (szSelect && (StringCompare(aszFiles.GetText(i), szSelect) == 0))
			{
				iSelect = mpList->NumRows();
			}
			AddRow(aszFiles.GetText(i), false);
		}
	}
	aszFiles.Kill();
	aszFolders.Kill();

	mpList->Select(iSelect);
	mpList->GetSelectionEvent()->Add(this, &CFileChooser::SelectionChanged);
}


//////////////////////////////////////////////////////////////////////////
//
// An icon and the folder's or file's name.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::AddRow(const char* szPath, bool bFolder)
{
	Ptr<CWindow>			pWindow;
	Ptr<CIconComponent>		pIcon;
	Ptr<CLabel>				pName;
	CFileUtil				cFileUtil;
	CChars					szName;
	int32					iRow;

	pWindow = mpDialog->GetWindow();
	iRow = mpList->AddRow(true);
	pIcon = OMalloc<CIconComponent>(pWindow, bFolder ? mpFolderIcon : mpFileIcon, FILE_CHOOSER_ICON_WIDTH, FILE_CHOOSER_ROW_HEIGHT);
	pIcon->SetCelAlignment(CSH_Center, CSV_Center);
	szName.Init();
	cFileUtil.GetFileName(&szName, (char*)szPath);
	pName = OMalloc<CLabel>(pWindow, mpDialog->GetLook()->pcTheme->GetFont(), szName.Text());
	pName->SetAlignment(CSH_Left, CSV_Center);
	szName.Kill();
	mpList->AddToRow(iRow, pIcon, 0);
	mpList->AddToRow(iRow, pName, 1, 1, 1);
	maszRows.Add(szPath);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the type chosen in Files of Type shows the file.
//
//////////////////////////////////////////////////////////////////////////
bool CFileChooser::IsShownFile(const char* szPath)
{
	CFileUtil	cFileUtil;
	const char*	szExtension;

	szExtension = GetTypeExtension();
	if (szExtension == NULL)
	{
		return true;
	}
	return cFileUtil.IsExtension(szPath, szExtension, true);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns true if a file was chosen.
//
//////////////////////////////////////////////////////////////////////////
bool CFileChooser::Show(void)
{
	return mpDialog->Show(FILE_CHOOSER_WIDTH, FILE_CHOOSER_HEIGHT);
}


//////////////////////////////////////////////////////////////////////////
//
// The whole path of the file chosen.
//
//////////////////////////////////////////////////////////////////////////
char* CFileChooser::GetChosen(void)
{
	return mszChosen.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// Open (or Save, or enter) chooses the file File Name names.  A folder is
// opened instead, and with File Name empty the selected folder is.  A
// file being opened must exist; one being saved over is chosen once the
// user says to replace it, on the chooser's next tick (AskReplace).
//
//////////////////////////////////////////////////////////////////////////
bool CFileChooser::Approve(void)
{
	CFileUtil	cFileUtil;
	CChars		szPath;
	char*		szText;
	const char*	szExtension;
	int32		iRow;

	szText = mpFileName->GetText();
	if (StrEmpty(szText))
	{
		iRow = mpList->GetSelected();
		if ((iRow >= 0) && (iRow < miNumFolders))
		{
			Open(maszRows.GetText(iRow), NULL);
		}
		return false;
	}

	if (cFileUtil.IsAbsolutePath(szText))
	{
		szPath.Init(szText);
	}
	else
	{
		szPath.Init(mszDirectory);
		cFileUtil.AppendToPath(&szPath, szText);
	}
	cFileUtil.FullPath(&szPath);

	if (cFileUtil.IsDirectory(szPath.Text()))
	{
		Open(szPath.Text(), NULL);
		mpFileName->SetText("");
		szPath.Kill();
		return false;
	}

	if (mbSave)
	{
		szExtension = GetTypeExtension();
		if (szExtension && !cFileUtil.IsExtension(szPath.Text(), szExtension, true))
		{
			szPath.Append('.');
			szPath.Append(szExtension);
		}
	}
	else if (!cFileUtil.Exists(szPath.Text()))
	{
		szPath.Kill();
		return false;
	}

	mszChosen.Set(szPath);
	szPath.Kill();
	if (mbSave && cFileUtil.Exists(mszChosen.Text()))
	{
		mbReplacePending = true;
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A file selected has its name put in File Name.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::SelectionChanged(void)
{
	CFileUtil	cFileUtil;
	CChars		szName;
	int32		iRow;

	iRow = mpList->GetSelected();
	if ((iRow < miNumFolders) || (iRow >= (int32)maszRows.NumElements()))
	{
		return;
	}
	szName.Init();
	cFileUtil.GetFileName(&szName, maszRows.GetText(iRow));
	mpFileName->SetText(szName.Text());
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A double click, or enter, on a folder opens it; on a file it chooses
// it.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::ItemActivated(void)
{
	int32	iRow;

	iRow = mpList->GetSelected();
	if ((iRow < 0) || (iRow >= (int32)maszRows.NumElements()))
	{
		return;
	}
	if (iRow < miNumFolders)
	{
		Open(maszRows.GetText(iRow), NULL);
	}
	else
	{
		mpDialog->OKClicked();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Look In opens the folder chosen.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::LookInChosen(void)
{
	int32	iIndex;

	iIndex = mpLookIn->GetSelected();
	if ((iIndex >= 0) && (iIndex < (int32)maszLookIn.NumElements()))
	{
		Open(maszLookIn.GetText(iIndex), NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Files of Type shows the folder's files of the type chosen.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::FileTypeChosen(void)
{
	FillList(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Up opens the parent folder, the folder come from selected.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::GoUp(void)
{
	CFileUtil	cFileUtil;
	CChars		szParent;

	if (!cFileUtil.IsRootDirectory(mszDirectory.Text()))
	{
		szParent.Init(mszDirectory);
		cFileUtil.RemoveLastFromPath(&szParent);
		Open(szParent.Text(), mszDirectory.Text());
		szParent.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// New Folder's dialog is shown on the next tick; it can't be shown from
// an input action, see CWindow::ShowModal.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::NewFolderClicked(void)
{
	mbNewFolderPending = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::RemoveFromButton(Ptr<CButton> pButton)
{
	if (pButton.IsNotNull())
	{
		pButton->GetClickEvent()->Remove(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Called by the chooser's tick, outside of its input.  Shows the New
// Folder dialog if New Folder was clicked, or asks whether to replace the
// file chosen to save.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::DialogTick(void)
{
	if (mbNewFolderPending)
	{
		mbNewFolderPending = false;
		NewFolder();
	}
	if (mbReplacePending)
	{
		mbReplacePending = false;
		AskReplace();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The user's home folder.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::GoHome(void)
{
	CFileUtil	cFileUtil;
	CChars		szHome;

	szHome.Init();
	if (cFileUtil.HomeDirectory(&szHome))
	{
		Open(szHome.Text(), NULL);
	}
	szHome.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Asks for the new folder's name and makes it in the folder shown, then
// selects it.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::NewFolder(void)
{
	CTextField*			pcName;
	CFileUtil			cFileUtil;
	CChars				szPath;
	CChars				szDirectory;

	//The chooser holds its New Folder dialog while it's shown, and the dialog holds its text field.
	mpNewFolderDialog = OMalloc<CDialog>(mpDialog->GetWindow(), mpDialog->GetLook(), "New Folder");
	pcName = &mpNewFolderDialog->AddTextField("Name", "New Folder");

	if (mpNewFolderDialog->Show(NEW_FOLDER_DIALOG_WIDTH) && !StrEmpty(pcName->GetText()))
	{
		szPath.Init(mszDirectory);
		cFileUtil.AppendToPath(&szPath, pcName->GetText());
		if (cFileUtil.MakeDir(szPath.Text()))
		{
			szDirectory.Init(mszDirectory);
			Open(szDirectory.Text(), szPath.Text());
			szDirectory.Kill();
		}
		szPath.Kill();
	}

	mpNewFolderDialog = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The file chosen to save exists: replacing it closes the chooser with
// it; otherwise the chooser stays open.
//
//////////////////////////////////////////////////////////////////////////
void CFileChooser::AskReplace(void)
{
	CFileUtil	cFileUtil;
	CChars		szName;
	CChars		szMessage;

	szName.Init();
	cFileUtil.GetFileName(&szName, mszChosen.Text());
	szMessage.Init(szName);
	szMessage.Append(" already exists.\nDo you want to replace it?");

	//The chooser holds the question while it's shown.
	mpReplaceDialog = OMalloc<CMessageDialog>(mpDialog->GetWindow(), mpDialog->GetLook(), "Confirm Save", szMessage.Text());
	mpReplaceDialog->GetDialog()->SetOKCaption("Replace");
	if (mpReplaceDialog->Show())
	{
		mpDialog->Close("OK");
	}
	mpReplaceDialog = NULL;

	szMessage.Kill();
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The extension of the type chosen in Files of Type, NULL for every file
// or if there are no types.
//
//////////////////////////////////////////////////////////////////////////
const char* CFileChooser::GetTypeExtension(void)
{
	int32	iType;

	iType = mpFileType->GetSelected();
	if ((iType < 0) || ((size)iType >= muiNumTypes))
	{
		return NULL;
	}
	return masTypes[iType].szExtension;
}

