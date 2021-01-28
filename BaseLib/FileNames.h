/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __FILE_NAMES_H__
#define __FILE_NAMES_H__
#include "ArrayChars.h"
#include "FileNode.h"


template <class M>
class CFileNames
{
protected:
	CFileNode<M>	mcRoot;
	BOOL			mbParentsValid;

public:
	void			Init(void);
	void			Kill(void);

	CFileNode<M>*	GetRoot(void);

	CFileNode<M>*	GetNode(char* szFullName);
	CFileNode<M>*	AddFile(char* szFullName);

	M*				GetNearestFile(char* szFullName, CChars* pszRemaining);
	void			FixParents(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CFileNames<M>::Init(void)
{
	mcRoot.InitDirectory("", NULL);
	mbParentsValid = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CFileNames<M>::Kill(void)
{
	mcRoot.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
CFileNode<M>* CFileNames<M>::GetRoot(void)
{
	return &mcRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
CFileNode<M>* CFileNames<M>::GetNode(char* szFullName)
{
	CChars			szFake;
	CArrayChars		asNames;
	int				i;
	char*			szName;
	CFileNode<M>*	pcCurrent;
	CFileNode<M>*	pcChild;

	asNames.Init();
	szFake.Fake(szFullName);
	szFake.Split(&asNames, '/');

	pcCurrent = &mcRoot;
	pcChild = NULL;
	for (i = 0; i < asNames.NumElements(); i++)
	{
		szName = asNames.GetText(i);
		if (pcCurrent->IsDirectory())
		{
			pcChild = pcCurrent->Directory()->Get(szName);
			if (pcChild == NULL)
			{
				break;
			}
		}
		else
		{
			pcChild = NULL;
			break;
		}
		pcCurrent = pcChild;
	}
	asNames.Kill();

	return pcChild;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
CFileNode<M>* CFileNames<M>::AddFile(char* szFullName)
{
	CChars			szFake;
	CArrayChars		asNames;
	int				i;
	char*			szName;
	CFileNode<M>*	pcCurrent;
	CFileNode<M>*	pcChild;

	mbParentsValid = FALSE;

	asNames.Init();
	szFake.Fake(szFullName);
	szFake.Split(&asNames, '/');

	pcCurrent = &mcRoot;
	pcChild = NULL;
	for (i = 0; i < asNames.NumElements(); i++)
	{
		szName = asNames.GetText(i);
		if (pcCurrent->IsDirectory())
		{
			pcChild = pcCurrent->Directory()->Get(szName);
		}
		else
		{
			//A file with this name already exists.
			pcChild = NULL;
			break;
		}
		if (pcChild == NULL)
		{
			if (i != asNames.NumElements() - 1)
			{
				pcChild = pcCurrent->Directory()->AddDirectory(szName, pcCurrent);
			}
			else
			{
				pcChild = pcCurrent->Directory()->AddFile(szName, pcCurrent);
			}
		}
		pcCurrent = pcChild;
	}
	asNames.Kill();

	return pcChild;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
M* CFileNames<M>::GetNearestFile(char* szFullName, CChars* pszRemaining)
{
	CChars			szFake;
	CArrayChars	asNames;
	int				i;
	CFileNode<M>*	pcCurrent;
	CFileNode<M>*	pcChild;
	int				iRemain;
	CChars*			szName;

	asNames.Init();
	szFake.Fake(szFullName);
	szFake.Split(&asNames, '/');

	pcCurrent = &mcRoot;
	pcChild = NULL;
	iRemain = 0;
	for (i = 0; i < asNames.NumElements(); i++)
	{
		szName = asNames.Get(i);
		if (pcCurrent->IsDirectory())
		{
			pcChild = pcCurrent->Directory()->Get(szName->Text());
			if (pcChild == NULL)
			{
				asNames.Kill();
				return NULL;
			}
		}
		else
		{
			pszRemaining->AppendSubString(szFullName, iRemain, szFake.Length());
			asNames.Kill();
			return pcCurrent->File();
		}
		iRemain += (szName->Length()+1);
		pcCurrent = pcChild;
	}

	asNames.Kill();
	if (pcCurrent->IsFile())
	{
		return pcCurrent->File();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CFileNames<M>::FixParents(void)
{
	if (!mbParentsValid)
	{
		mcRoot.Directory()->FixParents(&mcRoot);
		mbParentsValid = TRUE;
	}
}


#endif // __FILE_NAMES_H__

