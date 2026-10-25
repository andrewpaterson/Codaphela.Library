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
#include "ComponentLibraries.h"


static SComponentLibraryEntry gasLogicGates[] =
{
	{ "AndGate", "AND Gate", "Output high when all its inputs are high.", "and logic gate" },
	{ "NandGate", "NAND Gate", "Output low when all its inputs are high.", "nand not and logic gate" },
	{ "OrGate", "OR Gate", "Output high when any of its inputs is high.", "or logic gate" },
	{ "NorGate", "NOR Gate", "Output low when any of its inputs is high.", "nor not or logic gate" },
	{ "XorGate", "XOR Gate", "Output high when an odd number of its inputs are high.", "xor exclusive or logic gate parity" },
	{ "XnorGate", "XNOR Gate", "Output high when an even number of its inputs are high.", "xnor exclusive nor logic gate equality" },
	{ "Buffer", "Buffer", "Drives its output to its input's level.", "buffer driver logic" },
	{ "Inverter", "Inverter", "Drives its output to the opposite of its input's level.", "inverter not logic" },
	{ "InverterPackage", "74LVC04", "Six inverters in one 74 series part, drawn as one block.", "74lvc04 74x04 hex inverter package part not" },
	{ "InverterUnit", "74LVC04 Unit", "One inverter of a 74 series part, placed on its own.", "74lvc04 74x04 hex inverter unit part not" },
};


static SComponentLibraryEntry gasSequential[] =
{
	{ "DTypeFlipFlop", "D-Type Flip Flop", "Latches its data input on the clock's rising edge, with set and reset.", "flip flop latch register d type memory" },
	{ "Counter", "Up Counter", "Counts up on the clock's rising edge, with reset, load and a terminal count.", "counter binary register" },
};


static SComponentLibraryEntry gasSources[] =
{
	{ "Clock", "Clock", "A square wave oscillator, with an inverted output if wanted.", "clock oscillator square wave" },
	{ "Constant", "Constant", "Drives its bus with a fixed value.", "constant value bus source" },
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
	LIBRARY("Logic Gates", "Combinational gates and buffers.", gasLogicGates),
	LIBRARY("Sequential", "Flip flops and counters.", gasSequential),
	LIBRARY("Sources", "Clocks, constants and supplies.", gasSources),
	LIBRARY("Instruments", "Probes and oscilloscopes.", gasInstruments),
	LIBRARY("Wiring", "Pins, splitters and tunnels.", gasWiring),
	LIBRARY("Microprocessors", "Microprocessors.", gasMicroprocessors),
	LIBRARY("Decorative", "Things that are only drawn.", gasDecorative),
};


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

