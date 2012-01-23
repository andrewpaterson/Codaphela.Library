/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "KeyboardKeyList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyboardKeyList::Init(void)
{
	maszKeys.Init(8);
	masKeys.Init(8);

	maszKeys.Add("Play/Pause");
	maszKeys.Add("Volume Down");
	maszKeys.Add("Volume Up");
	maszKeys.Add("Mute Sound");
	maszKeys.Add("Left Control");
	maszKeys.Add("Left Special");
	maszKeys.Add("Left Alt");
	maszKeys.Add("Space");
	maszKeys.Add("Right Alt");
	maszKeys.Add("Right Special");
	maszKeys.Add("Menu");
	maszKeys.Add("Right Control");
	maszKeys.Add("Cursor Left");
	maszKeys.Add("Cursor Down");
	maszKeys.Add("Cursor Right");
	maszKeys.Add("Numpad 0");
	maszKeys.Add("Numpad .");
	maszKeys.Add("Esc");
	maszKeys.Add("F1");
	maszKeys.Add("F2");
	maszKeys.Add("F3");
	maszKeys.Add("F4");
	maszKeys.Add("F5");
	maszKeys.Add("F6");
	maszKeys.Add("F7");
	maszKeys.Add("F8");
	maszKeys.Add("F9");
	maszKeys.Add("F10");
	maszKeys.Add("F11");
	maszKeys.Add("F12");
	maszKeys.Add("PrtScn");
	maszKeys.Add("ScrLk");
	maszKeys.Add("Pause");
	maszKeys.Add("Calculator");
	maszKeys.Add("`");
	maszKeys.Add("1");
	maszKeys.Add("2");
	maszKeys.Add("3");
	maszKeys.Add("4");
	maszKeys.Add("5");
	maszKeys.Add("6");
	maszKeys.Add("7");
	maszKeys.Add("8");
	maszKeys.Add("9");
	maszKeys.Add("0");
	maszKeys.Add("-");
	maszKeys.Add("=");
	maszKeys.Add("Backspace");
	maszKeys.Add("Insert");
	maszKeys.Add("Home");
	maszKeys.Add("Page Up");
	maszKeys.Add("Num Lock");
	maszKeys.Add("Num Lock");
	maszKeys.Add("Numpad /");
	maszKeys.Add("Numpad *");
	maszKeys.Add("Numpad -");
	maszKeys.Add("Tab");
	maszKeys.Add("Q");
	maszKeys.Add("W");
	maszKeys.Add("E");
	maszKeys.Add("R");
	maszKeys.Add("T");
	maszKeys.Add("Y");
	maszKeys.Add("U");
	maszKeys.Add("I");
	maszKeys.Add("O");
	maszKeys.Add("P");
	maszKeys.Add("[");
	maszKeys.Add("]");
	maszKeys.Add("\\");
	maszKeys.Add("Delete");
	maszKeys.Add("End");
	maszKeys.Add("Page Down");
	maszKeys.Add("Numpad 7");
	maszKeys.Add("Numpad 8");
	maszKeys.Add("Numpad 9");
	maszKeys.Add("Numpad +");
	maszKeys.Add("Caps Lock");
	maszKeys.Add("Caps Lock");
	maszKeys.Add("A");
	maszKeys.Add("S");
	maszKeys.Add("D");
	maszKeys.Add("F");
	maszKeys.Add("G");
	maszKeys.Add("H");
	maszKeys.Add("J");
	maszKeys.Add("K");
	maszKeys.Add("L");
	maszKeys.Add(";");
	maszKeys.Add("'");
	maszKeys.Add("Enter");
	maszKeys.Add("Numpad 4");
	maszKeys.Add("Numpad 5");
	maszKeys.Add("Numpad 6");
	maszKeys.Add("Left Shift");
	maszKeys.Add("Z");
	maszKeys.Add("X");
	maszKeys.Add("C");
	maszKeys.Add("V");
	maszKeys.Add("B");
	maszKeys.Add("N");
	maszKeys.Add("M");
	maszKeys.Add(",");
	maszKeys.Add(".");
	maszKeys.Add("/");
	maszKeys.Add("Right Shift");
	maszKeys.Add("Cursor Up");
	maszKeys.Add("Numpad 1");
	maszKeys.Add("Numpad 2");
	maszKeys.Add("Numpad 3");
	maszKeys.Add("Numpad Enter");
	maszKeys.Add("Fuck");

	miCurrent = -1;
	Next();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyboardKeyList::Kill(void)
{
	maszKeys.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CKeyboardKeyList::GetCurrent(void)
{
	return maszKeys.Get(miCurrent)->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyboardKeyList::Next(void)
{
	SWinKey*	psWinKey;

	if (miCurrent < maszKeys.NumElements())
	{
		mbFirst = TRUE;
		miCurrent++;
		psWinKey = masKeys.Add();
		memset(psWinKey, 0, sizeof(SWinKey));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyboardKeyList::Key(unsigned int uiVkey, unsigned int uiFlags)
{
	CChars	szEvent;

	szEvent.Init();
	if (mbFirst)
	{
		mbFirst = FALSE;
		Pressed(uiVkey, uiFlags);
		szEvent.Append("Pressed: ");
		szEvent.Append(GetCurrent());
		szEvent.AppendNewLine();
	}   
	else
	{
		Released(uiVkey, uiFlags);
		szEvent.Append("Released: ");
		szEvent.Append(GetCurrent());
		Next();
		szEvent.Append(" (Next: ");
		szEvent.Append(GetCurrent());
		szEvent.Append(")\n");
	}
	szEvent.Dump();
	szEvent.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyboardKeyList::Pressed(unsigned int uiVKey, unsigned int uiFlags)
{
	SWinKey*	psWinKey;

	psWinKey = masKeys.Tail();
	psWinKey->uiVKey = uiVKey;
	psWinKey->uiPressedFlags = uiFlags;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyboardKeyList::Released(unsigned int uiVKey, unsigned int uiFlags)
{
	SWinKey*	psWinKey;

	psWinKey = masKeys.Tail();
	if (psWinKey->uiVKey != uiVKey)
	{
		return;
	}
	psWinKey->uiReleasedFlags= uiFlags;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyboardKeyList::ToString(CChars* psz)
{
	SWinKey*	psKey;
	int			i;

	psz->Append("\
<Device Type=\"WinRaw\">\n\
  <Category>Keyboard</Category>\n\
  <ID>WinRAW Keyboard: Name(Terminal Server Keyboard Driver), Type(00000000), SubType(00000000), KeyboardMode(00000001), NumberOfFunctionKeys(0000000C), NumberOfIndicators(00000003), NumberOfKeysTotal(00000065)</ID>\n\
  <Friendly>Terminal Server Keyboard</Friendly>\n\
  <Sources Type=\"Defined\">\n\
    <Format Type=\"Named\">WinRAW Keyboard</Format>\n");

	for (i = 0; i < masKeys.NumElements()-1; i++)
	{
		psKey = masKeys.Get(i);

		psz->Append("      <Source Type=\"State\">\n");
		psz->Append("        <Friendly>"); psz->Append(maszKeys.Get(i)); psz->Append("</Friendly>\n");
		psz->Append("        <State>\n");
		psz->Append("          <Channel Name=\"VKey\">\n");
		psz->Append("            <Equals>"); psz->Append(psKey->uiVKey); psz->Append("</Equals>\n");
		psz->Append("          </Channel>\n");
		psz->Append("          <Channel Name=\"Flags\">\n");
		psz->Append("            <Equals>"); psz->Append(psKey->uiPressedFlags); psz->Append("</Equals>\n");
		psz->Append("          </Channel>\n");
		psz->Append("          <Value Type=\"Numeric\">1.0</Value>\n");
		psz->Append("        </State>\n");
		psz->Append("        <State>\n");
		psz->Append("          <Channel Name=\"VKey\">\n");
		psz->Append("            <Equals>"); psz->Append(psKey->uiVKey); psz->Append("</Equals>\n");
		psz->Append("          </Channel>\n");
		psz->Append("          <Channel Name=\"Flags\">\n");
		psz->Append("            <Equals>"); psz->Append(psKey->uiReleasedFlags); psz->Append("</Equals>\n");
		psz->Append("          </Channel>\n");
		psz->Append("          <Value Type=\"Numeric\">0.0</Value>\n");
		psz->Append("        </State>\n");
		psz->Append("      </Source>\n");
	}

	psz->Append("\
  </Sources>\n\
</Device>\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyboardKeyList::Dump(void)
{
	CChars		sz;

	sz.Init();
	ToString(&sz);
	sz.Dump();
	sz.Kill();
}

