/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "LiteralHelper.h"
#include "CPPNameCharacters.h"


//A range of characters, both ends in it.
struct SCharacterRange
{
	uint32	uiFirst;
	uint32	uiLast;
};


//The characters past ASCII an identifier may have.  [charname.allowed]
static const SCharacterRange	gasAllowedInNames[] =
{
	{ 0x00A8, 0x00A8 }, { 0x00AA, 0x00AA }, { 0x00AD, 0x00AD }, { 0x00AF, 0x00AF }, { 0x00B2, 0x00B5 }, { 0x00B7, 0x00BA },
	{ 0x00BC, 0x00BE }, { 0x00C0, 0x00D6 }, { 0x00D8, 0x00F6 }, { 0x00F8, 0x00FF },
	{ 0x0100, 0x167F }, { 0x1681, 0x180D }, { 0x180F, 0x1FFF },
	{ 0x200B, 0x200D }, { 0x202A, 0x202E }, { 0x203F, 0x2040 }, { 0x2054, 0x2054 }, { 0x2060, 0x206F },
	{ 0x2070, 0x218F }, { 0x2460, 0x24FF }, { 0x2776, 0x2793 }, { 0x2C00, 0x2DFF }, { 0x2E80, 0x2FFF },
	{ 0x3004, 0x3007 }, { 0x3021, 0x302F }, { 0x3031, 0x303F },
	{ 0x3040, 0xD7FF },
	{ 0xF900, 0xFD3D }, { 0xFD40, 0xFDCF }, { 0xFDF0, 0xFE44 }, { 0xFE47, 0xFFFD },
	{ 0x10000, 0x1FFFD }, { 0x20000, 0x2FFFD }, { 0x30000, 0x3FFFD }, { 0x40000, 0x4FFFD }, { 0x50000, 0x5FFFD },
	{ 0x60000, 0x6FFFD }, { 0x70000, 0x7FFFD }, { 0x80000, 0x8FFFD }, { 0x90000, 0x9FFFD }, { 0xA0000, 0xAFFFD },
	{ 0xB0000, 0xBFFFD }, { 0xC0000, 0xCFFFD }, { 0xD0000, 0xDFFFD }, { 0xE0000, 0xEFFFD },
};


//Those of them an identifier may not start with (combining characters).  [charname.disallowed]
static const SCharacterRange	gasNotFirstInNames[] =
{
	{ 0x0300, 0x036F }, { 0x1DC0, 0x1DFF }, { 0x20D0, 0x20FF }, { 0xFE20, 0xFE2F },
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsInRanges(uint32 uiCharacter, const SCharacterRange* pasRanges, size uiNumRanges)
{
	size	i;

	for (i = 0; i < uiNumRanges; i++)
	{
		if ((uiCharacter >= pasRanges[i].uiFirst) && (uiCharacter <= pasRanges[i].uiLast))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The length of the universal character name at sz (\u and four
// hexadecimal digits or \U and eight), reading no further than szEnd; 0 if
// there isn't one.  [lex.charset] 2
//
//////////////////////////////////////////////////////////////////////////
size UniversalCharacterNameLength(const char* sz, const char* szEnd)
{
	size	uiLength;
	size	i;
	char	c;

	if ((szEnd - sz < 6) || (sz[0] != '\\') || ((sz[1] != 'u') && (sz[1] != 'U')))
	{
		return 0;
	}
	uiLength = (sz[1] == 'u') ? 6 : 10;
	if ((size)(szEnd - sz) < uiLength)
	{
		return 0;
	}
	for (i = 2; i < uiLength; i++)
	{
		c = sz[i];
		if (!(((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))))
		{
			return 0;
		}
	}
	return uiLength;
}


//////////////////////////////////////////////////////////////////////////
//
// What is wrong with the name szName (uiLength bytes of UTF-8, as the
// preprocessor makes every identifier), for an error; NULL if nothing.
// Its characters past ASCII must be ones the language allows in an
// identifier, and the first not a combining character.  [lex.name]
//
//////////////////////////////////////////////////////////////////////////
const char* InvalidCPPNameCharacter(const char* szName, size uiLength)
{
	const char*	sz;
	const char*	szEnd;
	const char*	szCharacter;
	uint32		uiCharacter;
	char		szUTF8[UTF8_MAX_BYTES];

	szEnd = szName + uiLength;
	for (sz = szName; sz < szEnd;)
	{
		if (*sz == '\\')
		{
			//Respell leaves a universal character name that may not be in a name as it was.  [lex.charset] 2
			return "A universal character name outside a literal doesn't name a basic, control or surrogate character: write the character itself";
		}
		if ((uint8)*sz < 0x80)
		{
			sz++;
			continue;
		}

		//Bytes that aren't a character's UTF-8 are read as one byte each: it isn't what the character is written as.
		szCharacter = sz;
		uiCharacter = DecodeUTF8(&sz, szEnd);
		if (EncodeUTF8(uiCharacter, szUTF8) != (size)(sz - szCharacter))
		{
			return "The source is not UTF-8 here";
		}
		if (!IsInRanges(uiCharacter, gasAllowedInNames, sizeof(gasAllowedInNames) / sizeof(SCharacterRange)))
		{
			return "This character is not allowed in an identifier";
		}
		if ((szCharacter == szName) && IsInRanges(uiCharacter, gasNotFirstInNames, sizeof(gasNotFirstInNames) / sizeof(SCharacterRange)))
		{
			return "An identifier can't start with this character";
		}
	}
	return NULL;
}

