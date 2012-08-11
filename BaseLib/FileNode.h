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
#ifndef __FILE_NODE_H__
#define __FILE_NODE_H__
#include "AbstractFile.h"
#include "Chars.h"
#include "FileNodeDirectory.h"
#include "PackFileNode.h"


enum EFileNodeType
{
	FNT_Unknown,
	FNT_Directory,
	FNT_File,
};


template <class M>
class CFileNode
{
friend class CFileNodeDirectory<M>;
protected:
	CChars			mszName;
	EFileNodeType	meType;
	CFileNode*		mpcParent;

	union
	{
		CFileNodeDirectory<M>	msDirectory;		
		M						msFile;
	} u;
	
public:
	void					InitFile(char* szName, CFileNode* pcParent);
	void					InitDirectory(char* szName, CFileNode* pcParent);
	void					Kill(void);

	char*					GetName(void);
	char*					GetFullName(CChars* pszDest);
	BOOL					Is(char* szName);
	BOOL					IsDirectory(void);
	BOOL					IsFile(void);
	CFileNodeDirectory<M>*	Directory(void);
	M*						File(void);
	CFileNode<M>*			GetParent(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CFileNode<M>::InitFile(char* szName, CFileNode* pcParent)
{
	meType = FNT_File;
	mszName.Init(szName);
	u.msFile.Init();
	mpcParent = pcParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CFileNode<M>::InitDirectory(char* szName, CFileNode* pcParent)
{
	meType = FNT_Directory;
	mszName.Init(szName);
	u.msDirectory.Init();
	mpcParent = pcParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CFileNode<M>::Kill(void)
{
	if (meType == FNT_File)
	{
		u.msFile.Kill();
	}
	else if (meType == FNT_Directory)
	{
		u.msDirectory.Kill();
	}

	mszName.Kill();
	meType = FNT_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
char* CFileNode<M>::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
char* CFileNode<M>::GetFullName(CChars* pszDest)
{
	CFileNode<M>*	pcCurrent;
	CChars			szName;

	szName.Init();

	pcCurrent = this;
	while (pcCurrent)
	{
		szName.Insert(0, pcCurrent->GetName());
		pcCurrent = pcCurrent->GetParent();
		szName.Insert(0, "/");
	}
	szName.RemoveFromStart(2);

	pszDest->Append(szName);
	szName.Kill();
	return pszDest->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
BOOL CFileNode<M>::Is(char* szName)
{
	return mszName.Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
BOOL CFileNode<M>::IsDirectory(void)
{
	return meType == FNT_Directory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
BOOL CFileNode<M>::IsFile(void)
{
	return meType == FNT_File;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
CFileNodeDirectory<M>* CFileNode<M>::Directory(void)
{
	return &u.msDirectory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
M* CFileNode<M>::File(void)
{
	return &u.msFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
CFileNode<M>* CFileNode<M>::GetParent(void)
{
	return mpcParent;
}


#endif // __FILE_NODE_H__

