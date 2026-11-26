# Numbers and Lines

Two value types that are BaseLib's own rather than the platform's: [`CNumber`], an arbitrary precision decimal kept
in packed BCD, and [`CLine`] with the classes around it, the integer grid geometry ported from Logicim for traces.

## CNumber

A [`CNumber`] is a decimal number of a fixed number of whole number digits and decimals, chosen when it is made: 26
and 16 by default (`DEFAULT_WHOLE_NUMBERS`, `DEFAULT_DECIMALS`).  Its header, [`CNumberDetail`], holds those two
counts, the positions of its first and last digits that aren't 0, and its flags (negative, not a number, overflow,
underflow, division by zero).  The digits follow it, so a number of another precision is a [`CNumber`] laid over
memory of `NUMBER_SIZE(whole, decimals)` bytes (the `UNumber` union).  The calculator ([`CCalculator`]), the type
parser and the markup's number attributes use it.

### Packed BCD

The digits are packed BCD in [`uint16`] words, [`CNumber::mauiDigits`]: four digits a word, the least significant
digit in the low nibble, the least significant word first.  The whole numbers and the decimals are each rounded up
to whole words (`NUMBER_WORD_DIGITS`), so the decimal point is on a word boundary and the same digit of two numbers
is in the same nibble of the same word, whatever their precisions.  [`CNumber::DigitPosition`] and
[`CNumber::PositionDigit`] convert between a digit (counted out from the decimal point) and its nibble, and
[`CNumber::MinimumPosition`] is the nibble of the smallest decimal.  Every nibble outside the number's first and
last digits, the spare nibbles of the rounded words included, is 0: the arithmetic relies on it.

The arithmetic is done a word at a time with [`BCDAdd`] and [`BCDSubtract`] ([`BaseLib/IntegerHelper.h`]).  On the
hosts and the 65C64 they are C++: each word is added in binary with 6 more in every nibble, so a nibble whose digits
sum to 10 or more carries out of it, and the 6 is taken off the nibbles that didn't carry.  On the W65C816 they are
macros for the compiler's `__builtin_bcd_add` and `__builtin_bcd_subtract`, which call runtime routines that loop
over the words in the processor's decimal mode.

| Operation | How |
|---|---|
| Add, subtract | The words that line up, then the carry or borrow rippled on up ([`CNumber::PrivateAddWords`], [`CNumber::PrivateRipple`]). |
| Multiply | The multiples of the number by 1 to 9, each made from the one before by one BCD add as the digits need them; each digit's multiple added in, shifted by nibbles to its power of ten ([`CNumber::PrivateAddPlaced`]). |
| Divide | Restoring division ([`CNumber::PrivateDivide`]): the divisor taken off until it borrows, then added back once. |
| Shift | Nibbles moved a word at a time, each word put together from the two it takes its nibbles from ([`CNumber::PrivateShiftNibbles`]). |
| Clean | The first and last digits that aren't 0 found by scanning words ([`CNumber::PrivateFindEnds`]). |
| Zeroing a range | Masks from tables ([`CNumber::PrivateZeroPositions`]): no shifts by a variable count, which the W65C816 does a bit at a time. |

Converting from text underflows only when a digit that is dropped isn't 0.  Adding a number with more decimals than
this one clips its extra decimals and sets underflow.

Measured on the W65C816 simulator, 2026-10-30: TestNumber took 70.3G instructions with a digit a char and 13.0G in
packed BCD.

## Line geometry

The classes in [`BaseLib/Line.h`] and its neighbours are Logicim's net.common.geometry package, ported for building
traces: lines on an integer grid, merged, split at junctions and found by the points they pass through.

| Class or function | Logicim | What it does |
|---|---|---|
| [`CLine`] ([`CArrayLine`]) | Line | A line between two [`SInt32Vec2`] points, a value class. |
| [`ELineDirection`] | Rotation (its line values) | Which way a line runs: [`LD_North`], [`LD_East`], [`LD_South`], [`LD_West`], or [`LD_Cannot`] for a line that isn't horizontal or vertical or has no length. |
| [`ELineOverlap`] | LineOverlap | How one line covers another: fully, its start, its end, its middle, crossing at right angles, or not at all. |
| [`CLineBucket`], [`MinimiseLines`] | LineBucket, LineMinimiser | Lines on the same horizontal or vertical merged into one covering them all. |
| [`CPositions`] | Positions | A set of grid points grouped by x, each kept once. |
| [`CLinePositionCache`] | LinePositionCache | Horizontal and vertical lines found by the points they pass through. |
| [`SplitLines`], [`CalculateLineJunctions`] | LineSplitter | Lines split where other lines' ends or given junctions are on them. |

A trace is made of horizontal and vertical lines; a line that is neither has direction [`LD_Cannot`] and never
overlaps or touches another.  The directions keep Logicim's names, which are not a map's: increasing y is
[`LD_North`] and increasing x is [`LD_West`].  Only the line values of Logicim's Rotation are ported, not the rest
of that enum, which is its user interface's.

Where the port differs from Logicim:

- Logicim's Line.createLine is [`CLine::InitOrthogonal`], which returns false for a line that isn't horizontal or
  vertical; Line.lines(...), a varargs maker, isn't ported.
- [`CLine::GetX`] and [`CLine::GetY`] on the wrong axis log an error instead of throwing, and return the start's.
- [`CLine::GetLength`] of a line that is neither horizontal nor vertical is its length rounded to a whole number
  (Logicim used the x length for both axes).
- The bounding box is in integers ([`CLine::GetBoundingBox`]); Logicim's was a Float2D.
- [`MinimiseLines`] and [`SplitLines`] are functions, not classes.  Where Logicim iterated a Set or a LinkedHashMap,
  the port keeps the same insertion order, so the lines come out in the same order.
- [`CPositions`] and [`CLinePositionCache`] find a column or a row by a [`CHashMapIntTemplate`] of indexes into
  arrays; the [`CLine`] pointers the cache returns are good only until it next changes.

## Decisions

- [`CNumber`] is packed BCD (the user, 2026-10-30, on being reminded that the W65C816 has a decimal mode), chosen
  over assembly helpers or a compiler idiom for the old layout of a digit a char; then "keep the packed BCD and
  continue tuning its C++".
- The line geometry is a port of Logicim's package into BaseLib (the user, 2026-10-23: "required for creating Traces
  and the only outstanding common classes"), keeping its names and its iteration orders.

## Not done

- Nothing outside the tests ([`Tests/TestBaseLib/TestBaseLib/TestLine.cpp`]) uses the line geometry yet: no trace is
  built with it.
