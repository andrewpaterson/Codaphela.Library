#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsSubStringEquals(void)
{
	CChars	sz;

	sz.Init("Happy");
	AssertTrue(sz.SubStringEquals(0, "Hap"));
	AssertTrue(sz.SubStringEquals(0, "Happy"));
	AssertTrue(sz.SubStringEquals(1, "app"));
	AssertTrue(sz.SubStringEquals(0, "H"));
	AssertTrue(sz.SubStringEquals(1, "a"));
	AssertTrue(sz.SubStringEquals(2, "p"));
	AssertTrue(sz.SubStringEquals(3, "p"));
	AssertTrue(sz.SubStringEquals(4, "y"));
	AssertTrue(sz.SubStringEquals(0, ""));

	AssertFalse(sz.SubStringEquals(1, "Hap"));
	AssertFalse(sz.SubStringEquals(0, "app"));
	AssertFalse(sz.SubStringEquals(6, ""));
	AssertFalse(sz.SubStringEquals(7, ""));
	AssertFalse(sz.SubStringEquals(5, "y"));
	sz.Kill();

	sz.Init();
	AssertFalse(sz.SubStringEquals(0, "Hap"));
	AssertFalse(sz.SubStringEquals(1, "Hap"));

	AssertTrue(sz.SubStringEquals(0, ""));
	AssertFalse(sz.SubStringEquals(1, ""));
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsReplace(void)
{
	CChars sz;

	sz.Init("1");
	sz.Overwrite(0, "22");
	AssertString("2", sz.Text());
	sz.Kill();

	sz.Init("Hello\nWorld\nMy Name is Henry\n");
	sz.Replace("\n", "[ ]");
	AssertString("Hello[ ]World[ ]My Name is Henry[ ]", sz.Text());
	sz.Replace("[ ]", "X");
	AssertString("HelloXWorldXMy Name is HenryX", sz.Text());
	sz.Replace("X", "*");
	AssertString("Hello*World*My Name is Henry*", sz.Text());
	sz.Replace("*", NULL);
	AssertString("HelloWorldMy Name is Henry", sz.Text());
	sz.Kill();

	sz.Init("XXZybot");
	sz.Replace("XX", "^");
	AssertString("^Zybot", sz.Text());
	sz.Replace("t", NULL);
	AssertString("^Zybo", sz.Text());
	sz.Replace("^", NULL);
	AssertString("Zybo", sz.Text());
	sz.Kill();

	sz.Init("XXZybot");
	sz.Replace("XX", "^^^^");
	AssertString("^^^^Zybot", sz.Text());
	sz.Replace("^^^^Zybot", "In the Age of Chaos");
	AssertString("In the Age of Chaos", sz.Text());
	sz.Replace("In the Age of Chaos", "Short");
	AssertString("Short", sz.Text());
	sz.Kill();

	sz.Init("SXSXSX56");
	sz.Replace("SX", "x");
	AssertString("xxx56", sz.Text());
	sz.Kill();

	sz.Init("   56");
	sz.Replace(" ", "");
	AssertString("56", sz.Text());
	sz.Kill();

	sz.Init("XXXXXXHelloXXXXXXXXXWorldXXXXXX");
	sz.Replace("XXX", "-");
	AssertString("--Hello---World--", sz.Text());
	sz.Kill();

	sz.Init("XXXXXXHelloXXXXXXXXXWorldXXXXXX");
	sz.Replace("XXX", "-+*+-");
	AssertString("-+*+--+*+-Hello-+*+--+*+--+*+-World-+*+--+*+-", sz.Text());
	sz.Kill();

	sz.Init("XXXXXXHelloXXXXXXXXXWorldXXXXXX");
	sz.Replace("XXX", "");
	AssertString("HelloWorld", sz.Text());
	sz.Kill();

	sz.Init("XXXXXXHelloXXXXXXXXXWorldXXXXXX");
	sz.Replace("XXX", NULL);
	AssertString("HelloWorld", sz.Text());
	sz.Kill();

	sz.Init("Q2");
	sz.Replace("Q2", "Queue");
	AssertString("Queue", sz.Text());
	sz.Kill();

	sz.Init("Q2");
	sz.Replace("Q2", "q");
	AssertString("q", sz.Text());
	sz.Kill();

	sz.Init("Q2Q2");
	sz.Replace("Q2", "");
	AssertString("", sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsReplaceIntensive(void)
{
	CChars	sz;
	CChars	szExpected;

	CChars	szOriginal;
	CChars	szReplacement;
	size	uiCount;
	size	uiCountCount;
	size	uiLength;
	
	szOriginal.Init();
	szOriginal.Append('B', 35);

	for (uiLength = 0; uiLength < 7; uiLength++)
	{
		for (uiCountCount = 0; uiCountCount < 20; uiCountCount++)
		{
			szExpected.Init();
			sz.Init();

			szReplacement.Init();
			szReplacement.Append('A', uiLength * 5);

			if (uiCountCount % 2 == 0)
			{
				for (uiCount = 0; uiCount < uiCountCount / 2; uiCount++)
				{
					szExpected.Append(szReplacement);
					szExpected.Append("xx");
					sz.Append(szOriginal);
					sz.Append("xx");
				}
				if (szExpected.Length() > 2)
				{
					szExpected.RemoveFromEnd(2);
					sz.RemoveFromEnd(2);
				}
			}
			else
			{
				for (uiCount = 0; uiCount < uiCountCount / 2; uiCount++)
				{
					szExpected.Append(szReplacement);
					szExpected.Append("xx");
					sz.Append(szOriginal);
					sz.Append("xx");
				}
			}

			sz.Replace(szOriginal.Text(), szReplacement.Text());
			szReplacement.Kill();

			AssertString(szExpected.Text(), sz.Text());
			sz.Kill();
			szExpected.Kill();
		}
	}
	szOriginal.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsInsert(void)
{
	CChars sz;

	sz.Init("Hell World");
	sz.Insert(4, "1234");
	AssertString("Hell1234 World", sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsSplit(void)
{
	CChars			sz;
	CArrayChars		cStrings;
	CChars*			psz;
	CChars			sz2;

	cStrings.Init();
	sz.Init("A.B");
	sz.Split(&cStrings, '.');
	sz.Kill();
	AssertSize(2, cStrings.NumElements());
	AssertString("A", cStrings.Get(0)->Text());
	AssertString("B", cStrings.Get(1)->Text());
	cStrings.Kill();
	
	sz.Init("\
Turburust BaseLib is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU Lesser General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.");
	sz2.Init(sz);

	cStrings.Init();
	sz.SplitLines(&cStrings);
	sz.Kill();

	AssertSize(4, cStrings.NumElements());
	AssertString("Turburust BaseLib is free software: you can redistribute it and/or modify", cStrings.Get(0)->Text());
	AssertString("it under the terms of the GNU Lesser General Public License as published by", cStrings.Get(1)->Text());
	AssertString("the Free Software Foundation, either version 3 of the License, or", cStrings.Get(2)->Text());
	AssertString("(at your option) any later version.", cStrings.Get(3)->Text());
	cStrings.Kill();

	cStrings.Fake();
	sz2.SplitLines(&cStrings);

	AssertSize(4, cStrings.NumElements());
	psz = cStrings.Get(0);
	AssertPointer(sz2.Text(), psz->Text());
	AssertSize(73, psz->Length());
	AssertBool(true, psz->Equals("Turburust BaseLib is free software: you can redistribute it and/or modify", 73));
	psz = cStrings.Get(1);
	AssertSize(75, psz->Length());
	AssertBool(true, psz->Equals("it under the terms of the GNU Lesser General Public License as published by", 75));
	psz = cStrings.Get(2);
	AssertSize(65, psz->Length());
	AssertBool(true, psz->Equals("the Free Software Foundation, either version 3 of the License, or", 65));
	psz = cStrings.Get(3);
	AssertSize(35, psz->Length());
	AssertBool(true, psz->Equals("(at your option) any later version.", 35));

	sz2.Kill();
	cStrings.Kill();

	sz.Init("\\Names\\Of\\Doom\\");
	cStrings.Init();
	sz.Split(&cStrings, '\\');
	sz.Kill();

	AssertSize(5, cStrings.NumElements());
	AssertString("", cStrings.Get(0)->Text());
	AssertString("Names", cStrings.Get(1)->Text());
	AssertString("Of", cStrings.Get(2)->Text());
	AssertString("Doom", cStrings.Get(3)->Text());
	AssertString("", cStrings.Get(4)->Text());
	cStrings.Kill();

	sz.Init("\\Names\\Of\\Doom\\");
	cStrings.Fake();
	sz.Split(&cStrings, '\\');

	AssertSize(5, cStrings.NumElements());

	psz = cStrings.Get(0);
	AssertSize(0, psz->Length());
	psz = cStrings.Get(1);
	AssertSize(5, psz->Length());
	psz = cStrings.Get(2);
	AssertSize(2, psz->Length());
	psz = cStrings.Get(3);
	AssertSize(4, psz->Length());
	psz = cStrings.Get(4);
	AssertSize(0, psz->Length());

	cStrings.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsMakeCPlusPlus(void)
{
	CChars sz;

	sz.Init("Hello World");
	sz.MakeCPlusPlus();
	AssertString("\"Hello World\"", sz.Text());
	sz.Kill();

	sz.Init("Hello World");
	sz.AppendNewLine();
	sz.Append("Goodbye");
	sz.MakeCPlusPlus();
	AssertString("\"Hello World\\n\"\n\"Goodbye\"", sz.Text());
	sz.Kill();

	sz.Init("Hello World");
	sz.AppendNewLine();
	sz.Append("Goodbye");
	sz.AppendNewLine();
	sz.MakeCPlusPlus();
	AssertString("\"Hello World\\n\"\n\"Goodbye\\n\"", sz.Text());
	sz.Kill();

	sz.Init("");
	sz.MakeCPlusPlus();
	AssertString("\"\"", sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsCopyEmptyString(void)
{
	CChars sz1;
	CChars sz2;

	sz1.Init();
	AssertTrue(sz1.Empty());
	AssertPointer(gszEmptyString, sz1.Text());
	
	sz2.Init();
	sz1.Append(sz2);
	AssertPointer(gszEmptyString, sz1.Text());
	AssertString("", sz1.Text());

	sz1.Append("Hello");
	AssertString("Hello", sz1.Text());
	
	sz2.Append(sz1);
	AssertString("Hello", sz2.Text());

	sz1.Kill();
	sz2.Kill();

	sz2.Init();
	sz1.Init("");
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init(' ', 0);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init(sz2);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init(&sz2);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init((char*)NULL);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init("Star", 4);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init("Star");
	AssertString("Star", sz1.Text());
	sz1.Kill();

	sz2.Kill();
	sz2.Init("Star");

	sz1.Init(sz2, 1, 3);
	AssertString("ta", sz1.Text());
	sz1.Kill();

	sz1.Init(sz2, 4, 4);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init("Star", 1, 3);
	AssertString("ta", sz1.Text());
	sz1.Kill();

	sz1.Init(' ', 2);
	AssertString("  ", sz1.Text());
	sz1.Kill();

	sz1.InitList((char*)NULL);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.InitList("Nova", NULL);
	AssertString("Nova", sz1.Text());
	sz1.Kill();

	sz1.InitList((CChars*)NULL);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.InitList(&sz2, NULL);
	AssertString("Star", sz1.Text());
	sz1.Kill();
	sz2.Kill();

	sz1.InitLength(0);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.InitLength(1);
	sz1.SetChar(0, 'A');
	AssertString("A", sz1.Text());
	sz1.Kill();

	sz1.InitData2("X", 0);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.InitData2(NULL, 1);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsInsertOutOfBounds(void)
{
	CChars sz1;
	CChars sz2;

	sz1.Init("It's Life");
	sz1.Insert(4, " My");
	AssertString("It's My Life", sz1.Text());

	sz1.Insert(12, '.');
	AssertString("It's My Life.", sz1.Text());

	sz2.Init("!!");

	sz1.Insert(14, &sz2);
	AssertString("It's My Life.", sz1.Text());

	sz1.Kill();
	sz2.Kill();

	sz1.Init();
	sz1.Insert(1, 'C');
	AssertPointer(gszEmptyString, sz1.Text());

	sz1.Insert(0, 'C');
	AssertString("C", sz1.Text());

	sz1.Insert(2, 'a');
	AssertString("C", sz1.Text());

	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsLeftAlign(void)
{
	CChars sz1;

	sz1.Init();
	sz1.LeftAlign("ABC", ' ', 6);
	AssertString("ABC   ", sz1.Text());
	sz1.Kill();

	sz1.Init();
	sz1.LeftAlign("ABC", ' ', 2);
	AssertString("AB", sz1.Text());
	sz1.Kill();

	sz1.Init();
	sz1.LeftAlign("ABC", ' ', 0);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init();
	sz1.LeftAlign("", ' ', 0);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsRightAlign(void)
{
	CChars sz1;

	sz1.Init();
	sz1.RightAlign("ABC", ' ', 6);
	AssertString("   ABC", sz1.Text());
	sz1.Kill();

	sz1.Init();
	sz1.RightAlign("ABC", ' ', 2);
	AssertString("AB", sz1.Text());
	sz1.Kill();

	sz1.Init();
	sz1.RightAlign("ABC", ' ', 0);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init();
	sz1.RightAlign("", ' ', 0);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsRemoveOutOfBounds(void)
{
	CChars sz1;

	sz1.Init();
	sz1.Remove(1, 2);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init("Mack");
	sz1.Remove(0, 3);
	AssertString("k", sz1.Text());
	sz1.Kill();

	sz1.Init("Mack");
	sz1.Remove(0, 4);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init("Mack");
	sz1.Remove(0, 2);
	AssertString("ck", sz1.Text());
	sz1.Kill();

	sz1.Init("Mack");
	sz1.Remove(2, 5);
	AssertString("Ma", sz1.Text());
	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsRemoveEnd(void)
{
	CChars sz1;

	sz1.Init();
	sz1.RemoveFromEnd(2);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init("Mack");
	sz1.RemoveFromEnd(1);
	AssertString("Mac", sz1.Text());

	sz1.RemoveFromEnd(3);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init();
	sz1.RemoveEnd(2);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init("Mack");
	sz1.RemoveEnd(3);
	AssertString("Mac", sz1.Text());

	sz1.RemoveEnd(0);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init("Mack");
	sz1.RemoveEnd(0);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();

	sz1.Init("Mack");
	sz1.RemoveFromEnd(5);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsRemoveCharacter(void)
{
	CChars sz1;

	sz1.Init();
	sz1.RemoveCharacter(0);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.RemoveCharacter(1);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();


	sz1.Init("Goblins");
	sz1.RemoveCharacter(0);
	AssertString("oblins", sz1.Text());
	sz1.RemoveCharacter(1);
	AssertString("olins", sz1.Text());
	sz1.RemoveCharacter(5);
	AssertString("olins", sz1.Text());
	sz1.RemoveCharacter(4);
	AssertString("olin", sz1.Text());
	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsFindOutOfBounds(void)
{
	CChars	sz1;
	size	iPos;

	sz1.Init("ABCDE");
	iPos = sz1.Find('C');
	AssertSize(2, iPos);

	iPos = sz1.Find(3, 'C');
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iPos);

	iPos = sz1.Find(5, 'C');
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iPos);

	iPos = sz1.Find(6, 'C');
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iPos);
	sz1.Kill();

	sz1.Init();
	iPos = sz1.Find('C');
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iPos);
	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsFindFromEndOutOfBounds(void)
{
	CChars	sz1;
	size	iPos;

	sz1.Init("ABCDE");
	iPos = sz1.FindFromEnd('C');
	AssertSize(2, iPos);

	iPos = sz1.FindFromEnd(1, 'C');
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iPos);

	iPos = sz1.FindFromEnd(0, 'C');
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iPos);

	iPos = sz1.FindFromEnd(6, 'C');
	AssertSize(2, iPos);
	sz1.Kill();

	sz1.Init();
	iPos = sz1.FindFromEnd('C');
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iPos);
	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsSetLength(void)
{
	CChars	sz1;

	sz1.Init();
	sz1.SetLength(0);
	AssertPointer(gszEmptyString, sz1.Text());

	sz1.SetLength(1);
	sz1.SetChar(0, 'A');
	AssertString("A", sz1.Text());

	sz1.SetLength(0);
	AssertPointer(gszEmptyString, sz1.Text());
	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsStripWhitespace(void)
{
	CChars	sz1;

	sz1.Init();
	sz1.StripWhitespace();
	AssertPointer(gszEmptyString, sz1.Text());

	sz1.Append("  Bork\t");
	sz1.StripWhitespace();
	AssertString("Bork", sz1.Text());

	sz1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCharsReadAndWrite(void)
{
	CChars			sz;
	CMemoryFile*	pcMemoryFile;
	CFileBasic		cFile;
	CChars			szInput;

	MemoryInit();

	pcMemoryFile = MemoryFile();
	cFile.Init(pcMemoryFile);
	cFile.Open(EFM_ReadWrite_Create);

	sz.Init("Controller");
	sz.WriteString(&cFile);
	sz.Kill();

	cFile.Close();
	
	cFile.Open(EFM_Read);
	szInput.ReadString(&cFile);
	cFile.Close();
	cFile.Kill();

	AssertString("Controller", szInput.Text());

	szInput.Kill();


	pcMemoryFile = MemoryFile();
	cFile.Init(pcMemoryFile);
	cFile.Open(EFM_ReadWrite_Create);

	sz.Init();
	AssertPointer(gszEmptyString, sz.Text());
	sz.WriteString(&cFile);
	sz.Kill();

	cFile.Close();

	cFile.Open(EFM_Read);
	szInput.ReadString(&cFile);
	cFile.Close();
	cFile.Kill();

	AssertString("", szInput.Text());
	AssertPointer(gszEmptyString, sz.Text());

	szInput.Kill();

	MemoryKill();
}


//////////////////////////////////////////////////////////////////////////
//
// CLargeChars is CChars counted with lsize; it appends natural strings.
//
//////////////////////////////////////////////////////////////////////////
void TestCharsLarge(void)
{
	CLargeChars		szLarge;
	CChars			szNatural;

	szLarge.Init("Hello");
	szNatural.Init(" World");
	szLarge.Append(&szNatural);
	szLarge.Append(szNatural);
	AssertString("Hello World World", szLarge.Text());
	AssertSize(17, szLarge.Length());
	AssertSize(6, szLarge.Find("World"));

	szLarge.Append('.', 70000);
	AssertLSize(70017, szLarge.Length());
	AssertChar('.', szLarge.GetChar(70016));
	AssertTrue(szLarge.EndsWith("..."));

	szLarge.Reverse();
	AssertLSize(70017, szLarge.Length());
	AssertTrue(szLarge.EndsWith("dlroW dlroW olleH"));
	AssertChar('.', szLarge.GetChar(0));
	AssertChar('\0', szLarge.Text()[70017]);

	szNatural.Clear();
	szNatural.Append("abc");
	szNatural.Reverse();
	AssertString("cba", szNatural.Text());
	szNatural.Clear();
	szNatural.Reverse();
	AssertString("", szNatural.Text());

	szNatural.Kill();
	szLarge.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestChars(void)
{
	BeginTests();

	TestCharsSubStringEquals();
	TestCharsReplace();
	TestCharsInsert();
	TestCharsSplit();
	TestCharsMakeCPlusPlus();
	TestCharsCopyEmptyString();
	TestCharsInsertOutOfBounds();
	TestCharsLeftAlign();
	TestCharsRightAlign();
	TestCharsRemoveOutOfBounds();
	TestCharsRemoveEnd();
	TestCharsRemoveCharacter();
	TestCharsFindOutOfBounds();
	TestCharsFindFromEndOutOfBounds();
	TestCharsSetLength();
	TestCharsStripWhitespace();
	TestCharsReadAndWrite();
	TestCharsReplaceIntensive();
	TestCharsLarge();
	
	TestStatistics();
}

