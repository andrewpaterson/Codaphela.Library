/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __C_P_P_SYNTAX_BLOCK_H__
#define __C_P_P_SYNTAX_BLOCK_H__
#include "BaseLib/ArrayUint8.h"
#include "CPPSyntax.h"


class CCPPSyntaxTree;
class CCPPScope;
class CCPPLazyLoader;


//What a block's declaration put in a scope outside it: a symbol of its own declared there (CCPPScope::Add), or a
//name of the scope that finds its symbol (CCPPScope::AddAlias, as an unscoped enum's enumerators are found).  A class
//is, as parsing does, the class of its name already there if there is one (uiReuse): a class definition's (as after
//class X;; it then has the definition and its members), or an elaborated declaration's (class X; or friend class X;).
#define CPP_SYNTAX_BLOCK_REUSE_DEFINITION	1
#define CPP_SYNTAX_BLOCK_REUSE_ELABORATED	2
struct SCPPSyntaxBlockDeclared
{
	CCPPScope*		pcScope;
	int64			lliName;
	CCPPSymbol*		pcSymbol;
	bool			bAlias;
	uint8			uiReuse;		//0, or CPP_SYNTAX_BLOCK_REUSE_DEFINITION or _ELABORATED.
};


typedef CArrayTemplate<SCPPSyntaxBlockDeclared>	CArraySyntaxBlockDeclared;


//A declaration of a syntax tree as a block of bytes, the tree's form on disk: its nodes (each kind's fields, as its
//VisitFields gives them), the scopes and symbols it declares, the symbols and scopes outside it that it refers to (by
//name: the global scope, the members of a symbol, a name in a scope, a class template's parameters), and what each of
//its tokens was found to be.
//Nodes, scopes and symbols are referred to by 16 bit indices within the block, and tokens from the declaration's
//first, so a block can be read into any tree with the same tokens anywhere.  Little endian, fixed widths.
//
//  u32 magic, u16 nodes, u16 strings, u16 externals, u16 scopes, u16 symbols, u16 tokens
//  kinds       u8 per node (so every node can be made before any record that refers to one is read)
//  strings     u16 length, the characters (names, and files')
//  externals   u8 kind: global scope; members of (u16 symbol); symbol (u16 scope, u16 name, and where its declaration
//              starts: u16 file's name, u32 line, u16 column);
//              parent of (u16 scope: a class template's parameters are its members' parent)
//  declared    u16 count + (u8 alias 1 | reuse << 1, u16 scope, u16 name, u16 symbol): what it put in scopes outside
//              it, in order: its own symbols declared there (added when read; a class reuses the class there), and
//              names there finding its symbols (aliases).  Before the scopes and symbols that may be a reused class.
//  scopes      u8 kind, u16 parent, u16 owner, u8 dependent, u16 qualifier, u16 count + bases, u16 count + usings,
//              u16 count + names (u16 name, u16 symbol)
//  symbols     u16 name, u8 kind, u16 declaration, u16 scope, u16 members, u16 next, u16 lazy declaration
//  nodes       u16 parent, u16 first token, u16 last token, then its fields
//  tokens      u16 symbol, u8 role
//
//A symbol or scope reference is an index in the block's own, or with CPP_SYNTAX_BLOCK_EXTERNAL its external's.  The
//block's own scopes are those its nodes have and the members of its own symbols; its own symbols those its nodes
//declare and those in its own scopes.  So an external is found only from externals before it.
#define CPP_SYNTAX_BLOCK_MAGIC		0x31425343		//"CSB1".
#define CPP_SYNTAX_BLOCK_NULL		0xFFFF
#define CPP_SYNTAX_BLOCK_OUTSIDE	0xFFFE			//A node outside the block (a link to one, or the declaration's parent).
#define CPP_SYNTAX_BLOCK_EXTERNAL	0x8000
#define CPP_SYNTAX_BLOCK_MOST		0x7FFF			//The most nodes, scopes, symbols, externals, strings or tokens.


//Why a declaration couldn't be written as a block (CCPPSyntaxBlockWriter::Write).
enum ECPPSyntaxBlock
{
	CPPSB_Written,
	CPPSB_TooBig,			//More than CPP_SYNTAX_BLOCK_MOST of something.
	CPPSB_UnnamedScope,		//It refers to a scope outside it that has no name (another declaration's function or block).
	CPPSB_UnnamedSymbol,	//It refers to a symbol outside it that isn't found by its name in its scope.
	CPPSB_Tokens,			//A node's token is outside the declaration's.
};


class CCPPSyntaxBlockWriter
{
public:
	static ECPPSyntaxBlock	Write(CCPPSyntaxTree* pcTree, CCPPSyntax* pcDeclaration, CCPPScope* pcGlobal, CArrayUint8* pauiDest, CArraySyntaxBlockDeclared* pasDeclared = NULL);
};


class CCPPSyntaxBlockReader
{
public:
	static CCPPSyntax*		Read(uint8* puiBlock, size uiLength, CCPPSyntaxTree* pcTree, int iFirstToken, CCPPScope* pcGlobal, CCPPSyntax* pcParent, CCPPLazyLoader* pcLoader, CArraySyntaxBlockDeclared* pasDeclared = NULL);
};


#endif // __C_P_P_SYNTAX_BLOCK_H__

