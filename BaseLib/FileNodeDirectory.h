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
#ifndef __FILE_NODE_DIRECTORY_H__
#define __FILE_NODE_DIRECTORY_H__
#include "ArrayBlock.h"

template <class M> 
class CFileNode;
template <class M>
class CFileNodeDirectory
{
friend class CPackFiles;
friend class CFileSystem;
protected:
	CArrayBlock		maNodeFiles;  //Array of CFileNode.

public:
	void 			Init(void);
	void 			Kill(void);

	CFileNode<M>*	Get(char* szName);
	CFileNode<M>*	AddFile(char* szName, CFileNode<M>* pcParent);
	CFileNode<M>*	AddDirectory(char* szName, CFileNode<M>* pcParent);

	void			FixParents(CFileNode<M>* pcContaintingNode);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CFileNodeDirectory<M>::Init(void)
{
	maNodeFiles.Init(sizeof(CFileNode<M>), 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CFileNodeDirectory<M>::Kill(void)
{
	int				i;
	CFileNode<M>*	pcChild;

	for (i = 0; i < maNodeFiles.NumElements(); i++)
	{
		pcChild = (CFileNode<M>*)maNodeFiles.Get(i);
		pcChild->Kill();
	}
	maNodeFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
CFileNode<M>* CFileNodeDirectory<M>::Get(char* szName)
{
	int				i;
	CFileNode<M>*	pcNode;

	for (i = 0; i < maNodeFiles.NumElements(); i++)
	{
		pcNode = (CFileNode<M>*)maNodeFiles.Get(i);
		if (pcNode->Is(szName))
		{
			return pcNode;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
CFileNode<M>* CFileNodeDirectory<M>::AddDirectory(char* szName, CFileNode<M>* pcParent)
{
	CFileNode<M>*	pcNode;

	pcNode = (CFileNode<M>*)maNodeFiles.Add();
	pcNode->InitDirectory(szName, pcParent);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
CFileNode<M>* CFileNodeDirectory<M>::AddFile(char* szName, CFileNode<M>* pcParent)
{
	CFileNode<M>*	pcNode;

	pcNode = (CFileNode<M>*)maNodeFiles.Add();
	pcNode->InitFile(szName, pcParent);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CFileNodeDirectory<M>::FixParents(CFileNode<M>* pcContaintingNode)
{
	int						i;
	CFileNode<M>*			pcNode;
	CFileNodeDirectory<M>*	pcDirectory;

	for (i = 0; i < maNodeFiles.NumElements(); i++)
	{
		pcNode = (CFileNode<M>*)maNodeFiles.Get(i);
		pcNode->mpcParent = pcContaintingNode;

		if (pcNode->IsDirectory())
		{
			pcDirectory = pcNode->Directory();
			pcDirectory->FixParents(pcNode);
		}
	}
}


#endif // __FILE_NODE_DIRECTORY_H__

