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
#ifndef __FILE_CHOOSER_H__
#define __FILE_CHOOSER_H__
#include "BaseLib/ArrayChars.h"
#include "ListContainer.h"
#include "ListListener.h"
#include "ComboBoxListener.h"
#include "ButtonListener.h"
#include "ScrollPane.h"
#include "Label.h"
#include "Dialog.h"


class CFileChooser;


//A Files of Type choice.
struct SFileChooserType
{
	const char*		szDescription;
	const char*		szExtension;  //Without the dot, NULL for every file.
};


//The chooser can't be every kind of listener itself, each is a CListener, so these pass what they hear on.
class CFileChooserListListener : public CListListener
{
CONSTRUCTABLE(CFileChooserListListener);
public:
	CFileChooser*	mpcChooser;

	void	Init(CFileChooser* pcChooser);
	void	SelectionChanged(CUnknown* pcSource, void* pvContext) override;
	void	ItemActivated(CUnknown* pcSource, void* pvContext) override;
};


class CFileChooserComboListener : public CComboBoxListener
{
CONSTRUCTABLE(CFileChooserComboListener);
public:
	CFileChooser*	mpcChooser;

	void	Init(CFileChooser* pcChooser);
	void	SelectionChanged(CUnknown* pcSource, void* pvContext) override;
};


class CFileChooserButtonListener : public CButtonListener
{
CONSTRUCTABLE(CFileChooserButtonListener);
public:
	CFileChooser*	mpcChooser;

	void	Init(CFileChooser* pcChooser);
	void	ButtonClicked(CUnknown* pcSource, void* pvContext) override;
};


//A load and save dialog, Swing's JFileChooser as its Metal look lays it out: Look In (Save In when saving)
//with the folders from the root down to the one shown, and Up One Level, Home and New Folder buttons, along the top;
//the folder's folders then its files under it; and File Name and Files of Type along the bottom.  Open (or Save)
//and Cancel are the buttons.
//
//Selecting a file puts its name in File Name.  A double click on a folder (or enter) opens it; on a file it chooses
//it.  Open chooses what File Name names, relative to the folder shown unless it's a whole path, but if that's a
//folder it opens it instead, as JFileChooser does.  A file that doesn't exist can't be opened.
//
//Saving with a type that has an extension, a name without it has it added.  The types are the caller's and must last
//as long as the chooser; the first is chosen to start with.
class CFileChooser : public CDialogApprover
{
protected:
	CDialog						mcDialog;
	Ptr<CComboBox>				mpLookIn;
	Ptr<CButton>				mpUp;
	Ptr<CButton>				mpHome;
	Ptr<CButton>				mpNewFolder;
	Ptr<CScrollPane>			mpScrollPane;
	Ptr<CListContainer>			mpList;
	Ptr<CTextField>				mpFileName;
	Ptr<CComboBox>				mpFileType;
	Ptr<CImageCel>				mpFolderIcon;
	Ptr<CImageCel>				mpFileIcon;
	CFileChooserListListener	mcListListener;
	CFileChooserComboListener	mcComboListener;
	CFileChooserButtonListener	mcButtonListener;
	CChars						mszDirectory;  //The folder shown, a whole path.
	CArrayChars					maszLookIn;  //Each of Look In's folders, the root first.
	CArrayChars					maszRows;  //Each row's whole path.
	int32						miNumFolders;  //The rows before the files.
	CChars						mszChosen;
	bool						mbSave;
	SFileChooserType*			masTypes;
	size						muiNumTypes;

public:
	void			Init(Ptr<CWindow> pOwner, SDialogLook* psLook, const char* szTitle, bool bSave, const char* szFilename, SFileChooserType* asTypes, size uiNumTypes, Ptr<CImageCel> pFolderIcon, Ptr<CImageCel> pFileIcon);
	void			Kill(void);

	bool			Show(void);
	char*			GetChosen(void);

	bool			Approve(void) override;
	void			SelectionChanged(void);
	void			ItemActivated(void);
	void			ComboChanged(CUnknown* pcSource);
	void			ButtonClicked(CUnknown* pcSource);

protected:
	Ptr<CComponent>	CreateBody(void);
	Ptr<CLabel>		CreateLabel(const char* szText, int32 iWidth);
	Ptr<CButton>	CreateButton(const char* szCaption);
	void			AddButton(Ptr<CGridContainer> pRow, Ptr<CButton> pButton, int iColumn, int32 iGap);
	bool			Open(const char* szDirectory, const char* szSelect);
	void			FillLookIn(void);
	void			FillList(const char* szSelect);
	void			AddRow(const char* szPath, bool bFolder);
	bool			IsShownFile(const char* szPath);
	const char*		GetTypeExtension(void);
	void			NewFolder(void);
	void			GoHome(void);
};


void	NormaliseDirectory(CChars* pszDirectory);


#endif // __FILE_CHOOSER_H__

