/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "BaseFileNode.h"
#include "FileNodeDirectory.h"
#include "PackFileNode.h"


template <class M>
class CFileNode : public CBaseFileNode
{
friend class CFileNodeDirectory<M>;
protected:
	union
	{
		CFileNodeDirectory<M>	msDirectory;		
		M						msFile;
	} u;
	
public:
	void					InitFile(const char* szName, CFileNode* pcParent);
	void					InitDirectory(const char* szName, CFileNode* pcParent);
	void					Kill(void);

	CFileNodeDirectory<M>*	Directory(void);
	M*						File(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CFileNode<M>::InitFile(const char* szName, CFileNode* pcParent)
{
	Init(szName, FNT_File, pcParent);
	u.msFile.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CFileNode<M>::InitDirectory(const char* szName, CFileNode* pcParent)
{
	Init(szName, FNT_Directory, pcParent);
	meType = FNT_Directory;
	u.msDirectory.Init();
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

	CBaseFileNode::Kill();
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


#endif // !__FILE_NODE_H__

