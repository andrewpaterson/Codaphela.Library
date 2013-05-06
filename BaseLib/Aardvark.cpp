/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifdef _MSC_VER

#include "Aardvark.h"

#define NEW_LINE "\n\r"
char gszNewLine[] = "Hoo" NEW_LINE "Yay!" NEW_LINE;
#undef NEW_LINE

char gszd9[] = D9("Hello", "World");

void DoStuff()
{
	//You can only legally have hashes in the replacement text.  Bizzare things happen otherwise.
	//You can only have a single hash preceding a replacement argument.
	//Tokens after the hash are not expanded before hashing.
	char sz6[] = D9("Hello", "World");
	char sz1[] = D7(Hello, World);  //Tick
	char sz3[] = D8(Hello, World);  //Tick
	char sz5[] = D1(Hello, World);  //Tick
	char sz4[] = D2(Hello, World);  //Tick
	char sz2[] = D6(Hello, World);
}

wchar_t gwsz[] = L"Hello World";
#include "Aardvark.h"

#define floa	int giJoe;
float	gfriend;
floa
#define SOME_FOO_0(ARG1, ARG2) SOME_FOO_1(ARG1, ARG2)
#define SOME_FOO_1(ARG1, ARG2)          0xDE + ARG1 - ARG2 +ARG1 //  ''"' for US
#define SOME_FOO_2        0xE2  //  "<>" or "\|" on RT 102-key kbd.
#define SOME_FOO_3         0x00000080L /* Don't do "&" character translation */
int dufi = SOME_FOO_1(1, 2), dasf = SOME_FOO_2, ewqr = SOME_FOO_3, duki = SOME_FOO_0(3, 5);

VERTICAL_INT	giVerticalInt;

char gsz[] = ("//@  Don't Collapse me please     ! /*     ");
char g_Char = '\n';
char g_Char2 = '"';
namespace geran
{
	namespace dariel
	{
		void DoStuff(void);
	}

	/*
	* idObject values for WinEventProc and NotifyWinEvent
	*/

	/*
	* hwnd + idObject can be used with OLEACC.DLL's OleGetObjectFromWindow()
	* to get an interface pointer to the container.  indexChild is the item
	* within the container in question.  Setup a VARIANT with vt VT_I4 and
	* lVal the indexChild and pass that in to all methods.  Then you
	* are raring to go.
	*/


	/*
	* Common object IDs (cookies, only for sending WM_GETOBJECT to get at the
	* thing in question).  Positive IDs are reserved for apps (app specific),
	* negative IDs are system things and are global, 0 means "just little old
	* me".
	*/

	template<class M>
	class CFood
	{
		operator float* ();
		M& operator[ ](int iElementPos);
		M& operator()(int iElementPos);
		void DoStuff(void);
	};
	#include "Aardvark.h"
}

template<class M>
void geran::CFood<M>::DoStuff(void) {}
void geran::dariel::DoStuff(void) {}

template<class M>
geran::CFood<M>::operator float* ();
template<class M>
geran::CFood<M>::operator float* () {};


void MoreFoo(void)
{
	((void*)-1 == (void*)+1);
	for(;;){}
	for (int i = 3, j = 0; i < 10; i++, j=j+1)	{	}

	void* pcCurr = 0;
	void* pcNext = 0;
	while ((pcCurr) && (pcCurr)) {};
};

void FooTheElipse(int iStop, ...);

//#include "stdlib.h"
//void Fooey(int (*Func)(const void *,const void *))
//{
//	int miUsedElements = 10;
//	void* pvArray = NULL;
//
//	qsort((void*)pvArray, miUsedElements, sizeof(int), Func);
//	qsort((void*)pvArray, miUsedElements, sizeof miUsedElements, Func);
//	pvArray = (void*)realloc(pvArray, miUsedElements * sizeof(int));
//}

#endif
