# Makes the modules that TestModPlayer reads.  Run from this directory:
#
#   python3 MakeMOD.py
#
# Every module (including the tune) is original and made here, so there is no
# licence to worry about.  The rows are documented with what the tests expect.

import math
import random
import struct


def period(note):
	"""ProTracker finetune 0 periods, C-1 is 'C-1'."""
	table = [856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
			 428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
			 214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113]
	names = ['C-', 'C#', 'D-', 'D#', 'E-', 'F-', 'F#', 'G-', 'G#', 'A-', 'A#', 'B-']
	return table[names.index(note[0:2]) + 12 * (int(note[2]) - 1)]


class Sample:
	def __init__(self, name, data, volume=64, finetune=0, loopStart=0, loopLength=0):
		self.name = name
		self.data = bytes(data)
		self.volume = volume
		self.finetune = finetune
		self.loopStart = loopStart		# Bytes; must be even.
		self.loopLength = loopLength	# Bytes; 0 for no loop.


def cell(sample=0, note=None, effect=0, param=0):
	return (sample, period(note) if isinstance(note, str) else (note or 0), effect, param)


def module(title, samples, orders, patterns, channels=4, signature=None, numSamples=31, restart=127):
	"""patterns[p][row] is a dictionary of channel to cell."""
	out = bytearray(title.encode('ascii')[:20].ljust(20, b'\0'))
	for i in range(numSamples):
		s = samples[i] if i < len(samples) else Sample('', b'')
		out += s.name.encode('ascii')[:22].ljust(22, b'\0')
		out += struct.pack('>HBBHH', len(s.data) // 2, s.finetune & 0x0F, s.volume, s.loopStart // 2, (s.loopLength // 2) if s.loopLength else 1)
	out += bytes([len(orders), restart])
	out += bytes(orders).ljust(128, b'\0')
	if numSamples == 31:
		out += (signature or {4: b'M.K.', 6: b'6CHN', 8: b'8CHN'}[channels])
	for pattern in patterns:
		for row in range(64):
			for channel in range(channels):
				sample, p, effect, param = pattern.get(row, {}).get(channel, (0, 0, 0, 0))
				out += bytes([(sample & 0xF0) | (p >> 8), p & 0xFF, ((sample & 0x0F) << 4) | effect, param])
	for s in samples[:numSamples]:
		out += s.data
	return bytes(out)


def write(name, data):
	with open(name, 'wb') as f:
		f.write(data)


def square(length, high=64):
	return [high if i < length // 2 else (256 - high) for i in range(length)]


def ramp(length):
	return [(i * 256 // length - 128) & 0xFF for i in range(length)]


# One looped square wave note on channel 0 at C-2, nothing else.  64 rows of 6 ticks of 882 frames at 44100 Hz.
def makeBasic():
	samples = [Sample('Square', square(32), loopStart=0, loopLength=32)]
	patterns = [{0: {0: cell(1, 'C-2')}}]
	write('Basic.mod', module('Basic', samples, [0], patterns))


# Effects.  Channel 0 unless noted; speed 6 until row 14.
#   0  C-2 s1 C20		volume 32
#   1  A04				volume slides down 4 a tick: 28 24 20 16 12
#   2  102				period 428 down 2 a tick: 426 ... 418
#   3  203				period up 3 a tick: 421 ... 433
#   4  C#2 304			tone portamento to 404 by 4: 429 425 421 417 413
#   5  300				continues: 409 405 404 404 404
#   6  047				arpeggio on 404 (C#2): tick 1 F-2 (320), tick 2 G#2 (269), tick 3 404 ...
#   7  E13				fine slide up 3 on tick 0: 401
#   8  448				vibrato speed 4 depth 8: out periods 401+0 +6 +11 +14 +15
#   9  748				tremolo on volume 12: 12 24 34 41 43
#  10  EC3				volume cut on tick 3
#  11  C-2 s1 ED2		volume 64 and period 428 on tick 0, the sample restarts on tick 2 (as ProTracker)
#  12  E93				retrigger on ticks 3 (position back to 0)
#  13  C-2 s2 901		sample offset 256
#  14  F03, ch1 840	speed 3, channel 1 pan -0.5
#  15  FFA				tempo 250: 441 frames a tick at 44100 Hz
#  16  EE2				pattern delay: the row lasts 3 x 3 ticks
#  17  E60				pattern loop start
#  18  E62				loop twice: rows 17 18 17 18 17 18
#  19  D05				break to the next order, row 5
# Pattern 1 (order 1):
#   5  B00				jump to order 0 row 0, which was played: the song ends
def makeEffects():
	samples = [Sample('Square', square(32), loopStart=0, loopLength=32),
			   Sample('Ramp', ramp(512))]
	p0 = {
		0: {0: cell(1, 'C-2', 0xC, 0x20)},
		1: {0: cell(0, None, 0xA, 0x04)},
		2: {0: cell(0, None, 0x1, 0x02)},
		3: {0: cell(0, None, 0x2, 0x03)},
		4: {0: cell(0, 'C#2', 0x3, 0x04)},
		5: {0: cell(0, None, 0x3, 0x00)},
		6: {0: cell(0, None, 0x0, 0x47)},
		7: {0: cell(0, None, 0xE, 0x13)},
		8: {0: cell(0, None, 0x4, 0x48)},
		9: {0: cell(0, None, 0x7, 0x48)},
		10: {0: cell(0, None, 0xE, 0xC3)},
		11: {0: cell(1, 'C-2', 0xE, 0xD2)},
		12: {0: cell(0, None, 0xE, 0x93)},
		13: {0: cell(2, 'C-2', 0x9, 0x01)},
		14: {0: cell(0, None, 0xF, 0x03), 1: cell(0, None, 0x8, 0x40)},
		15: {0: cell(0, None, 0xF, 0xFA)},
		16: {0: cell(0, None, 0xE, 0xE2)},
		17: {0: cell(0, None, 0xE, 0x60)},
		18: {0: cell(0, None, 0xE, 0x62)},
		19: {0: cell(0, None, 0xD, 0x05)},
	}
	p1 = {5: {0: cell(0, None, 0xB, 0x00)}}
	write('Effects.mod', module('Effects', samples, [0, 1], [p0, p1]))


# Finetune: sample 1 has finetune -3, sample 2 +7.  Row 0 C-2 s1 (period 437), row 1 C-2 s2 (407), row 2 C-2 s1 E55 (413).
def makeFineTune():
	samples = [Sample('Minus3', square(32), finetune=-3, loopStart=0, loopLength=32),
			   Sample('Plus7', square(32), finetune=7, loopStart=0, loopLength=32)]
	p0 = {0: {0: cell(1, 'C-2')}, 1: {0: cell(2, 'C-2')}, 2: {0: cell(1, 'C-2', 0xE, 0x55)}, 3: {0: cell(0, None, 0xF, 0x00)}}
	write('FineTune.mod', module('FineTune', samples, [0], [p0]))


# 8 channels, a note on each.
def makeEightChannels():
	samples = [Sample('Square', square(32), loopStart=0, loopLength=32)]
	p0 = {0: {c: cell(1, 'C-2') for c in range(8)}, 1: {0: cell(0, None, 0xF, 0x00)}}
	write('Eight.mod', module('Eight', samples, [0], [p0], channels=8))


# A 15 sample Soundtracker module: one note.
def makeSoundtracker():
	samples = [Sample('Square', square(32), loopStart=0, loopLength=32)]
	p0 = {0: {0: cell(1, 'C-2')}, 1: {0: cell(0, None, 0xF, 0x00)}}
	write('Soundtracker.mod', module('Soundtracker', samples, [0], [p0], numSamples=15))


def makeBad():
	good = module('Bad', [Sample('Square', square(32))], [0], [{}])
	write('Truncated.mod', good[:1500])
	write('NoSong.mod', good[:950] + b'\0' + good[951:])
	write('Garbage.mod', bytes(random.Random(1).randrange(256) for _ in range(3000)))


makeBasic()
makeEffects()
makeFineTune()
makeEightChannels()
makeSoundtracker()
makeBad()


# An original tune: A minor, Am F C G, 125 BPM.  Channel 0 drums, 1 bass, 2 lead, 3 arpeggiated chords.
def synth(length, f, decay=None, loop=False):
	data = []
	for i in range(length):
		v = f(i)
		if decay:
			v *= math.exp(-i / decay)
		data.append(int(max(-128, min(127, round(v * 127)))) & 0xFF)
	return data


def makeTune():
	rnd = random.Random(1979)
	rate = 8287.0	# C-2
	kickPhase = [0.0]

	def kick(i):
		t = i / rate
		kickPhase[0] += 2 * math.pi * (45 + 110 * math.exp(-t * 30)) / rate
		return math.sin(kickPhase[0])

	samples = [
		Sample('Kick', synth(1800, kick, decay=450)),
		Sample('Snare', synth(1700, lambda i: 0.75 * rnd.uniform(-1, 1) + 0.35 * math.sin(2 * math.pi * 190 * i / rate), decay=380), volume=52),
		Sample('Hat', synth(500, lambda i: rnd.uniform(-1, 1) * (1 if i % 2 else -1), decay=90), volume=30),
		Sample('Bass', synth(64, lambda i: 1.0 - 2.0 * i / 64), volume=56, loopStart=0, loopLength=64),
		Sample('Lead', synth(32, lambda i: 0.8 if i < 8 else -0.8), volume=40, loopStart=0, loopLength=32),
		Sample('Chord', synth(32, lambda i: 1.0 - 4.0 * abs(i / 32 - 0.5)), volume=34, loopStart=0, loopLength=32),
	]
	KICK, SNARE, HAT, BASS, LEAD, CHORD = 1, 2, 3, 4, 5, 6
	chords = [('A-2', 0x37, 'A-1'), ('F-2', 0x47, 'F-1'), ('C-2', 0x47, 'C-2'), ('G-2', 0x47, 'G-1')]

	def rhythm(p, roll=False):
		for bar in range(4):
			b = bar * 16
			for r, s in ((0, KICK), (4, SNARE), (8, KICK), (10, KICK), (12, SNARE)):
				p.setdefault(b + r, {})[0] = cell(s, 'C-2')
			for r in (2, 6, 14):
				p.setdefault(b + r, {})[0] = cell(HAT, 'C-2')
		if roll:
			for r in range(56, 64):
				p.setdefault(r, {})[0] = cell(SNARE, 'C-2', 0xE, 0x93 if r >= 60 else 0x00)
		return p

	def backing(p, tremolo=False):
		for bar, (chord, arp, root) in enumerate(chords):
			b = bar * 16
			for r in range(16):
				cellsBass = p.setdefault(b + r, {})
				if r % 2 == 0:
					cellsBass[1] = cell(BASS, root if r % 4 == 0 else root.replace('1', '2') if root.endswith('1') else root.replace('2', '3'))
				cellsChord = p.setdefault(b + r, {})
				if r == 0:
					cellsChord[3] = cell(CHORD, chord, 0x0, arp)
				elif tremolo and r >= 8:
					cellsChord[3] = cell(0, None, 0x7, 0x64)
				else:
					cellsChord[3] = cell(0, None, 0x0, arp)
		return p

	def lead(p, melody, slide=False):
		for row, note, length in melody:
			p.setdefault(row, {})[2] = cell(LEAD, note, 0x3 if slide else 0, 0x10 if slide else 0)
			for r in range(row + 1, row + length):
				if slide and r < row + 3:
					p.setdefault(r, {})[2] = cell(0, None, 0x3, 0x00)
				elif length >= 6 and r >= row + 3:
					p.setdefault(r, {})[2] = cell(0, None, 0x6 if r == row + length - 1 else 0x4, 0x02 if r == row + length - 1 else 0x46)
		return p

	melody = [(0, 'E-3', 6), (6, 'D-3', 2), (8, 'C-3', 4), (12, 'B-2', 4),
			  (16, 'A-2', 6), (22, 'C-3', 2), (24, 'F-3', 8),
			  (32, 'E-3', 4), (36, 'G-3', 4), (40, 'E-3', 4), (44, 'C-3', 4),
			  (48, 'D-3', 8), (56, 'B-2', 4), (60, 'G-2', 4)]
	p0 = backing(rhythm({}))
	p0.setdefault(0, {})[1] = cell(BASS, 'A-1', 0xF, 0x06)
	p1 = lead(backing(rhythm({})), melody)
	p2 = lead(backing(rhythm({}, roll=True), tremolo=True), melody, slide=True)
	write('Tune.mod', module('Codaphela Tune', samples, [0, 1, 2, 1], [p0, p1, p2], restart=0))


makeTune()
