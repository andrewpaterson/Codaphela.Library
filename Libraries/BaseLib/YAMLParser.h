/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __YAML_PARSER_H__
#define __YAML_PARSER_H__
#include "Markup.h"
#include "Logger.h"
#include "BaseParserString.h"
#include "BaseParser.h"


//Reads a YAML document into a CMarkup tree, the tree CJSONParser makes (YAML's data is JSON's): a mapping is a tag
//whose scalar values are attributes and whose mappings and sequences are child tags, named by their keys; a sequence
//is a tag whose scalars are unnamed attributes and whose mappings and sequences are unnamed child tags.  The root is a
//tag named "" (no root for an empty document).
//
//Plain scalars are typed as YAML 1.2's core schema: null (~, null, empty), true and false, numbers (decimal digits with
//an optional sign and fraction; hexadecimal, octal, exponents, .inf and .nan stay strings), else strings.  Quoted
//scalars are strings.  A tagged scalar (!!python/name:x.y) is kept as text: the tag, and a space and the value if
//there is one.
//
//Supported: block mappings and sequences (nested, compact "- key: value" entries, sequences at their key's indent),
//flow [ ] and { } collections (over several lines too), single and double quoted scalars (escapes included), comments,
//one document with an optional --- and ... .  Refused, with an error naming the line: anchors (&) and aliases (*),
//block scalars (| and >), complex keys (?), values continuing on the next line, tags on collections, directives (%),
//more than one document, tabs as indentation and duplicate keys.
class CYAMLParser : public CBaseParser
{
public:
	CMarkupDoc*			mpcDoc;

	void		Init(char* szText, lsize iTextLen, const char* szFileName, CMarkup* pcMarkup, CLogger* pcLogger);
	void		Init(char* szText, lsize iTextLen, const char* szFileName, CMarkupDoc* pcDoc, CLogger* pcLogger);
	void		Init(const char* szText, const char* szFileName, CMarkup* pcMarkup, CLogger* pcLogger);
	void		Init(const char* szText, const char* szFileName, CMarkupDoc* pcDoc, CLogger* pcLogger);
	void		Kill(void);

	TRISTATE	Parse(void);

protected:
	TRISTATE	ParseRoot(void);
	TRISTATE	ParseBlockNode(const char* szName, lsize uiParentIndent, bool bSameIndentSequence);
	TRISTATE	ParseBlockMapping(const char* szName, lsize uiIndent);
	TRISTATE	ParseBlockSequence(const char* szName, lsize uiIndent);
	TRISTATE	ParseInlineValue(const char* szName);
	TRISTATE	ParseTagged(const char* szName);
	TRISTATE	ParseFlowSequence(const char* szName);
	TRISTATE	ParseFlowMapping(const char* szName);
	TRISTATE	ParseFlowNode(const char* szName);
	TRISTATE	ParseBlockKey(CChars* pszKey);
	TRISTATE	ParseFlowKey(CChars* pszKey);
	TRISTATE	ParseQuoted(CChars* pszValue);
	TRISTATE	ParseDoubleQuoted(CChars* pszValue);
	TRISTATE	ParseSingleQuoted(CChars* pszValue);
	TRISTATE	ParseEscape(CChars* pszValue);
	TRISTATE	ParsePlain(CChars* pszValue, bool bFlow);
	TRISTATE	AddScalar(const char* szName, const char* szPlain);
	TRISTATE	AddTag(const char* szName);
	void		EndTag(void);
	TRISTATE	CheckKey(const char* szKey);
	TRISTATE	CheckIndicator(void);
	TRISTATE	SkipToContent(void);
	TRISTATE	EndLine(void);
	TRISTATE	SkipFlowWhitespace(void);

	bool		IsLineEnd(void);
	bool		IsSequenceEntry(void);
	bool		IsKeyAhead(void);
	bool		IsDocumentMarker(const char* szMarker);
	char		Peek(lsize uiOffset);
	void		Step(lsize uiCount);
};


bool IsYAMLSeparator(char c);
bool IsYAMLFlowIndicator(char c);
bool IsYAMLNumber(const char* szText);


#endif // __YAML_PARSER_H__

