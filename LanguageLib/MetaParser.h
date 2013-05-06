/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __METAPARSER_H__
#define __METAPARSER_H__

/*  How this works:
*  
*  A defintion is declared with the $ character prefixing the definition name.
*  The one exception is the 'main' definition which is always the first text in the file and is not named.
*  
*  A node can be explicitly created by using parenthesis.  Elements in a node can be separated using one
*  of three separators.  The | character chooses only one of the elements in the node eg: (A | B).
*  
*  The & character parses if all the elements parse.  It is only valid with the ! modifier and is used to
*  disallow elements eg: (A & !B & !C).
*
*  A 'space' - ie: no separator - means the elements must parse in order eg: (A B C).  If any elements
*  fail then the node fails.
*
*  The ! character modifies element by logically negating the parse result.  A node with a ! modified element
*  will fail if the element parses normally (before negation).
*
*  A node may be followed by the + or - characters.  The + indicates that the node will not be simplified
*  eg: ((A)+) will remain ((A)) and not collapse to (A).  The - means the contents of the element should
*  be discarded and not added into the token tree.  The node is still created as necessary.
*
*  Following the + or - is an optional node name.  eg: (A)+MyName or (B)-OtherName.  It is possible for the
*  - node to be collapsed at which point the name is lost.
*
*  A nodes repetition can be set by the pattern ?-?.  eg: (A) 0-1 makes the node A optional.  (B) 1-1 makes B
*  compulsory.  If ommited the node is 1-1.  The right hand digit may be replaced by and X which implies no
*  limit.  eg:  (C) 0-X means C can be repeated as often as needed or not be present at all.
*
*  The . character is the last node option and stops the token tree from rolling back past it.  
*  eg:  (C | (A B).)	This is useful for error reporting as otherwise the entire parse tree will rollback 
*  and confusingly report the root node as being in error.
*
*  An element can be a string literal eg: "Hello" or character literal eg: 'c'.  An element can also be one
*  of the primitive keywords: integer, real, string or char.  A string must be surrounded by double quotes.  
*  eg: "Yo!" but places no constraint on what the string contains.
*
*  A character must be surrounded by single quotes eg: '$'.  Escape codes can be used as normal in both
*  strings and chars.
*  
*  An element can be the keyword 'identifier' which parses c style identifiers:  eg: iIndex or _Node38 but
*  not 74Id or $%.
*
*  Lastly an element can be a definition.  eg:  $DefinitionA can be used as ("Hello" DefinitionA).  This
*  replaces 'DefinitionA' with whatever the node $DefinitionA actually contained.
*  
*/

#include "CoreLib/Textparser.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "Meta.h"
#include "NodeConstructor.h"

class CMetaParser
{
public:
	CMeta*				mpcMeta;
	CTextParser*		mpcParser;
	CDefinition			mcCurrentDefinition;

	void 		Init(CMeta* pcMeta);
	TRISTATE 	Init(CMeta* pcMeta, char* szLanguageDefinition);
	void 		Kill(void);

	TRISTATE	Parse(char* szLanguageDefinition);
	TRISTATE	CreateDefinitionHolders(void);
	TRISTATE	ParseDefinition(CDefinition* pcDefinition);

	TRISTATE	ParseNode(int* piNodeDepth);
	TRISTATE	ParseCompoundNode(EElement eInverse, int* piNodeDepth);
	TRISTATE	ParseCompoundNodeOptions(CNodeConstructor* pcNode);
	TRISTATE	ParseSimpleNode(EElement eInverse);
	TRISTATE	ParseCharacter(void);
	TRISTATE	ParseString(void);
	TRISTATE	ParseUnknownIdentifier(void);
	TRISTATE	ParseEndOfNode(CNodeConstructor* pcNode);
	TRISTATE	ParseEndOfNodeRepetition(CNodeConstructor* pcNode);
	TRISTATE	ParseEndOfNodeNecessity(CNodeConstructor* pcNode);
	TRISTATE	ParseEndOfNodeFinallity(CNodeConstructor* pcNode);
	TRISTATE	ParseKnownDefinition(void);
	TRISTATE	ParseKeyword(void);
	TRISTATE	ParseEnd(void);

	void		GetIncompleteDefinitions(CChars* sz);
};


#endif //__METAPARSER_H__

