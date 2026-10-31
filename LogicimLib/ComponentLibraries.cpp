/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <ctype.h>
#include <stdio.h>
#include "BaseLib/Chars.h"
#include "Logic74Parts.h"
#include "ComponentPlacement.h"
#include "ComponentLibraries.h"


//The most entries the 74 series library can have: a package and a unit for each part.
#define LOGIC_74_LIBRARY_ENTRIES	256
#define LOGIC_74_LIBRARY_TEXT		128


static SComponentLibraryEntry gasGenericLogic[] =
{
	{ "AndGate", "AND Gate", "Output high when all its inputs are high.", "and logic gate" },
	{ "NandGate", "NAND Gate", "Output low when all its inputs are high.", "nand not and logic gate" },
	{ "OrGate", "OR Gate", "Output high when any of its inputs is high.", "or logic gate" },
	{ "NorGate", "NOR Gate", "Output low when any of its inputs is high.", "nor not or logic gate" },
	{ "XorGate", "XOR Gate", "Output high when an odd number of its inputs are high.", "xor exclusive or logic gate parity" },
	{ "XnorGate", "XNOR Gate", "Output high when an even number of its inputs are high.", "xnor exclusive nor logic gate equality" },
	{ "Buffer", "Buffer", "Drives its output to its input's level.", "buffer driver logic" },
	{ "Inverter", "Inverter", "Drives its output to the opposite of its input's level.", "inverter not logic" },
	{ "BusBuffer", "Bus Buffer", "Three state buffers in groups, each group with its own active low output enables.", "buffer bus three state tri line driver enable" },
	{ "BusTransceiver", "Bus Transceiver", "Drives either of two buses from the other, as its direction says.", "transceiver bus bidirectional three state" },
};


//Made from the 74 series parts (see Logic74Parts.h) the first time the libraries are looked at.
struct SLogic74LibraryText
{
	char	szXMLName[LOGIC_74_LIBRARY_TEXT];
	char	szName[LOGIC_74_LIBRARY_TEXT];
	char	szDescription[LOGIC_74_LIBRARY_TEXT];
	char	szKeywords[LOGIC_74_LIBRARY_TEXT];
};
static SComponentLibraryEntry	gas74Series[LOGIC_74_LIBRARY_ENTRIES];
static SLogic74LibraryText		gas74SeriesText[LOGIC_74_LIBRARY_ENTRIES];
static bool						gb74SeriesMade = false;


static SComponentLibraryEntry gasMemory[] =
{
	{ "DTypeFlipFlop", "D-Type Flip Flop", "Latches its data input on the clock's rising edge, with set and reset.", "flip flop latch register d type memory" },
	{ "Counter", "Up Counter", "Counts up on the clock's rising edge, with reset, load and a terminal count.", "counter binary register" },
	{ "Register", "Register", "Takes its data on the clock's rising edge and drives it while its output enable is low.", "register flip flop edge three state octal" },
	{ "Latch", "Latch", "Follows its data while latch enable is high, holds it when it goes low, and drives it while its output enable is low.", "latch transparent three state octal" },
};


static SComponentLibraryEntry gasPlexers[] =
{
	{ "Multiplexer", "Multiplexer", "Each channel drives the input its select bits choose.", "multiplexer mux selector data" },
	{ "Decoder", "Decoder", "Drives the active low output its address picks low.", "decoder demultiplexer demux address" },
};


static SComponentLibraryEntry gasArithmetic[] =
{
	{ "Adder", "Adder", "Adds two numbers and a carry in, with the carry out.", "adder sum carry arithmetic binary" },
	{ "Comparator", "Comparator", "Drives P = Q low while its two numbers are equal.", "comparator equality identity arithmetic" },
};


static SComponentLibraryEntry gasSources[] =
{
	{ "Clock", "Clock", "A square wave oscillator, with an inverted output if wanted.", "clock oscillator square wave" },
	{ "Constant", "Constant", "Drives its bus with a fixed value.", "constant value bus source" },
};


static SComponentLibraryEntry gasPower[] =
{
	{ "PositivePower", "Positive Power", "A supply at a fixed voltage.", "power supply vcc voltage" },
	{ "Ground", "Ground", "A supply at 0V.", "ground gnd 0v power" },
};


static SComponentLibraryEntry gasInstruments[] =
{
	{ "Probe", "Probe", "Shows the value on its bus.", "probe value bus display" },
	{ "Oscilloscope", "Oscilloscope", "Draws the voltages on its inputs over time.", "oscilloscope scope trace voltage time" },
};


static SComponentLibraryEntry gasWiring[] =
{
	{ "Pin", "Pin", "A subcircuit's connection to the circuit holding an instance of it.", "pin port subcircuit io" },
	{ "Splitter", "Splitter", "Splits a bus into narrower buses and joins them again.", "splitter bus join split" },
	{ "Tunnel", "Tunnel", "Joins its wire to every other tunnel of the same name.", "tunnel label net name" },
};


static SComponentLibraryEntry gasMicroprocessors[] =
{
	{ "W65C816", "W65C816", "The Western Design Center 16 bit microprocessor.", "w65c816 65816 cpu mpu microprocessor wdc" },
	{ "MPU65C64", "65C64", "A 6502 style microprocessor with 64 bit registers.", "65c64 cpu mpu microprocessor" },
};


static SComponentLibraryEntry gasDecorative[] =
{
	{ "Label", "Label", "Text on the circuit, boxed and filled if wanted.", "label text note comment" },
};


#define LIBRARY(szName, szDescription, asEntries)	{ szName, szDescription, asEntries, sizeof(asEntries) / sizeof(SComponentLibraryEntry) }


static SComponentLibrary gasComponentLibraries[] =
{
	LIBRARY("Generic Logic", "Combinational gates and buffers of any width.", gasGenericLogic),
	{ "74' Series", "74 series logic parts, as packages and as their units.", gas74Series, 0 },
	LIBRARY("Memory", "Flip flops, counters, registers and latches.", gasMemory),
	LIBRARY("Sources", "Clocks and constants.", gasSources),
	LIBRARY("Power", "Supplies and ground.", gasPower),
	LIBRARY("Plexers", "Multiplexers and decoders.", gasPlexers),
	LIBRARY("Arithmetic", "Adders and comparators.", gasArithmetic),
	LIBRARY("Instruments", "Probes and oscilloscopes.", gasInstruments),
	LIBRARY("Wiring", "Pins, splitters and tunnels.", gasWiring),
	LIBRARY("Microprocessors", "Microprocessors.", gasMicroprocessors),
	LIBRARY("Decorative", "Things that are only drawn.", gasDecorative),
};


//////////////////////////////////////////////////////////////////////////
//
// An entry placing szXMLName's view of the part ("Logic74Package:00")
// shown as "74LVC00", or "74LVC00 Unit" for a unit.
//
//////////////////////////////////////////////////////////////////////////
static void AddLogic74LibraryEntry(SComponentLibrary* psLibrary, SLogic74Part* psPart, const char* szXMLName, bool bUnit)
{
	SComponentLibraryEntry*	psEntry;
	SLogic74LibraryText*	psText;
	SLogic74PartType		sType;
	CChars					sz;

	if (psLibrary->uiNumEntries >= LOGIC_74_LIBRARY_ENTRIES)
	{
		return;
	}
	psEntry = &gas74Series[psLibrary->uiNumEntries];
	psText = &gas74SeriesText[psLibrary->uiNumEntries];
	psLibrary->uiNumEntries++;

	snprintf(psText->szXMLName, LOGIC_74_LIBRARY_TEXT, "%s%c%s", szXMLName, PLACED_PART_SEPARATOR, psPart->szPartType);
	snprintf(psText->szName, LOGIC_74_LIBRARY_TEXT, "74LVC%s%s", psPart->szPartType, bUnit ? " Unit" : "");
	FindLogic74Part(&sType, psPart->szPartType);
	sz.Init();
	GetLogic74Description(&sz, &sType);
	if (bUnit)
	{
		snprintf(psText->szDescription, LOGIC_74_LIBRARY_TEXT, "One unit of a %s, placed on its own.", sz.Text());
	}
	else
	{
		snprintf(psText->szDescription, LOGIC_74_LIBRARY_TEXT, "%s, drawn as one block.", sz.Text());
	}
	sz.Kill();
	snprintf(psText->szKeywords, LOGIC_74_LIBRARY_TEXT, "74lvc%s 74x%s %s %s", psPart->szPartType, psPart->szPartType, psPart->szKeywords, bUnit ? "unit" : "package");

	psEntry->szXMLName = psText->szXMLName;
	psEntry->szName = psText->szName;
	psEntry->szDescription = psText->szDescription;
	psEntry->szKeywords = psText->szKeywords;
}


//////////////////////////////////////////////////////////////////////////
//
// Each part's package, then its first unit if it has more than one.
//
//////////////////////////////////////////////////////////////////////////
static void MakeLogic74Library(SComponentLibrary* psLibrary)
{
	size			i;
	SLogic74Part*	psPart;

	if (gb74SeriesMade)
	{
		return;
	}
	gb74SeriesMade = true;
	psLibrary->uiNumEntries = 0;
	for (i = 0; i < NumLogic74Parts(); i++)
	{
		psPart = GetLogic74Part(i);
		AddLogic74LibraryEntry(psLibrary, psPart, "Logic74Package", false);
		if (psPart->iUnitCount > 1)
		{
			AddLogic74LibraryEntry(psLibrary, psPart, "Logic74Unit", true);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size NumComponentLibraries(void)
{
	return sizeof(gasComponentLibraries) / sizeof(SComponentLibrary);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentLibrary* GetComponentLibrary(size uiIndex)
{
	if (uiIndex >= NumComponentLibraries())
	{
		return NULL;
	}
	if (gasComponentLibraries[uiIndex].asEntries == gas74Series)
	{
		MakeLogic74Library(&gasComponentLibraries[uiIndex]);
	}
	return &gasComponentLibraries[uiIndex];
}


//////////////////////////////////////////////////////////////////////////
//
// As KiCad's symbol chooser filters: every word of the filter is found,
// ignoring case, in the component's name, description or keywords or its
// library's name.  An empty filter matches everything.
//
//////////////////////////////////////////////////////////////////////////
bool ComponentMatchesFilter(SComponentLibrary* psLibrary, SComponentLibraryEntry* psEntry, const char* szFilter)
{
	char		szWord[64];
	size		uiLength;
	const char*	szPosition;

	szPosition = szFilter;
	for (;;)
	{
		while (*szPosition == ' ')
		{
			szPosition++;
		}
		if (*szPosition == '\0')
		{
			return true;
		}
		uiLength = 0;
		while ((*szPosition != ' ') && (*szPosition != '\0'))
		{
			if (uiLength < sizeof(szWord) - 1)
			{
				szWord[uiLength] = *szPosition;
				uiLength++;
			}
			szPosition++;
		}
		szWord[uiLength] = '\0';

		if (!TextMatchesFilter(psEntry->szName, szWord) && !TextMatchesFilter(psEntry->szDescription, szWord) &&
			!TextMatchesFilter(psEntry->szKeywords, szWord) && !TextMatchesFilter(psLibrary->szName, szWord))
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the word is in the text, ignoring case.
//
//////////////////////////////////////////////////////////////////////////
bool TextMatchesFilter(const char* szText, const char* szWord)
{
	size	i;
	size	j;

	if (szText == NULL)
	{
		return false;
	}
	for (i = 0; szText[i] != '\0'; i++)
	{
		for (j = 0; (szWord[j] != '\0') && (szText[i + j] != '\0'); j++)
		{
			if (tolower((unsigned char)szText[i + j]) != tolower((unsigned char)szWord[j]))
			{
				break;
			}
		}
		if (szWord[j] == '\0')
		{
			return true;
		}
	}
	return szWord[0] == '\0';
}

