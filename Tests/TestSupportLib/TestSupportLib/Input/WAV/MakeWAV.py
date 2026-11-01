# Makes the WAVE files that TestSoundWAV reads.  Run from this directory:
#
#   python3 MakeWAV.py
#
# The files are built byte by byte here (and one with Python's wave module) so
# the reader is tested against files it did not write.  The sample patterns
# must match the Expected* functions in TestSoundWAV.cpp.

import os
import struct
import wave

GUID_TAIL = bytes([0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71])


def expected8(i, c):
	return (i * 7 + c * 50) & 0xFF


def expected16(i, c):
	return ((i * 1237 + c * 9001) & 0xFFFF) - 32768


def expected24(i, c):
	return ((i * 76543 + c * 1000003) & 0xFFFFFF) - 0x800000


def expected32(i, c):
	return ((i * 123456789 + c * 987654321) & 0xFFFFFFFF) - 0x80000000


def expectedFloat32(i, c):
	return ((i * 37 + c * 11) % 200 - 100) / 128.0


def expectedFloat64(i, c):
	return ((i * 12345 + c * 777) % 100000 - 50000) / 65536.0


def samples(bits, isFloat, frames, channels):
	data = bytearray()
	for i in range(frames):
		for c in range(channels):
			if isFloat and bits == 32:
				data += struct.pack('<f', expectedFloat32(i, c))
			elif isFloat:
				data += struct.pack('<d', expectedFloat64(i, c))
			elif bits == 8:
				data += struct.pack('<B', expected8(i, c))
			elif bits == 16:
				data += struct.pack('<h', expected16(i, c))
			elif bits == 24:
				data += struct.pack('<i', expected24(i, c))[0:3]
			else:
				data += struct.pack('<i', expected32(i, c))
	return bytes(data)


def chunk(id, body, size=None):
	if size is None:
		size = len(body)
	data = id + struct.pack('<I', size) + body
	if len(body) & 1:
		data += b'\0'
	return data


def fmt(tag, channels, rate, bits, blockAlign=None):
	if blockAlign is None:
		blockAlign = channels * (bits // 8)
	return struct.pack('<HHIIHH', tag, channels, rate, rate * blockAlign, blockAlign, bits)


def fmtExtensible(subFormat, channels, rate, bits, validBits, mask, guidTail=GUID_TAIL):
	return fmt(0xFFFE, channels, rate, bits) + struct.pack('<HHIH', 22, validBits, mask, subFormat) + guidTail


def riff(chunks, form=b'WAVE', id=b'RIFF'):
	body = form + b''.join(chunks)
	return id + struct.pack('<I', len(body)) + body


def write(name, data):
	with open(name, 'wb') as f:
		f.write(data)


def writeGood():
	#8 bit mono with an odd sized data chunk (padded) and a LIST chunk after the data.
	write('pcm8mono.wav', riff([chunk(b'fmt ', fmt(1, 1, 8000, 8)), chunk(b'data', samples(8, False, 101, 1)), chunk(b'LIST', b'INFOISFT\x04\x00\x00\x00Test')]))

	#16 bit stereo with an odd sized chunk between the format and the data.
	write('pcm16stereo.wav', riff([chunk(b'fmt ', fmt(1, 2, 44100, 16)), chunk(b'junk', b'odd!!'), chunk(b'data', samples(16, False, 1000, 2))]))

	#24 bit stereo without WAVE_FORMAT_EXTENSIBLE, which many programs write.
	write('pcm24stereo.wav', riff([chunk(b'fmt ', fmt(1, 2, 48000, 24)), chunk(b'data', samples(24, False, 500, 2))]))

	#32 bit front centre.
	write('pcm32centre.wav', riff([chunk(b'fmt ', fmtExtensible(1, 1, 96000, 32, 32, 0x4)), chunk(b'data', samples(32, False, 300, 1))]))

	#Float stereo with a fact chunk.
	write('float32stereo.wav', riff([chunk(b'fmt ', fmt(3, 2, 22050, 32) + struct.pack('<H', 0)), chunk(b'fact', struct.pack('<I', 200)), chunk(b'data', samples(32, True, 200, 2))]))

	#Extensible float64 with no channel mask.
	write('float64mono.wav', riff([chunk(b'fmt ', fmtExtensible(3, 1, 11025, 64, 64, 0)), chunk(b'fact', struct.pack('<I', 150)), chunk(b'data', samples(64, True, 150, 1))]))

	#5.1.
	write('pcm16surround51.wav', riff([chunk(b'fmt ', fmtExtensible(1, 6, 48000, 16, 16, 0x3F)), chunk(b'data', samples(16, False, 100, 6))]))

	#24 valid bits in a 32 bit container.
	data = bytearray()
	for i in range(100):
		for c in range(2):
			data += struct.pack('<i', expected24(i, c) << 8)
	write('pcm24in32.wav', riff([chunk(b'fmt ', fmtExtensible(1, 2, 48000, 32, 24, 0x3)), chunk(b'data', bytes(data))]))

	#Four channels without a mask are quadraphonic.
	write('pcm16quad.wav', riff([chunk(b'fmt ', fmt(1, 4, 44100, 16)), chunk(b'data', samples(16, False, 50, 4))]))

	#A mask with fewer speakers than channels.  The last channel is discrete.
	write('pcm16discrete.wav', riff([chunk(b'fmt ', fmtExtensible(1, 3, 44100, 16, 16, 0x3)), chunk(b'data', samples(16, False, 60, 3))]))

	#A streamed file with an unknown data size and a partial frame at the end.
	data = samples(16, False, 77, 1) + b'\x01'
	write('streamed.wav', b'RIFF' + struct.pack('<I', 0xFFFFFFFF) + b'WAVE' + chunk(b'fmt ', fmt(1, 1, 16000, 16)) + b'data' + struct.pack('<I', 0xFFFFFFFF) + data)

	#Written by Python's wave module.
	with wave.open('wavemodule.wav', 'wb') as w:
		w.setnchannels(2)
		w.setsampwidth(2)
		w.setframerate(32000)
		w.writeframes(samples(16, False, 400, 2))


def writeBad():
	os.makedirs('Bad', exist_ok=True)
	good = [chunk(b'fmt ', fmt(1, 1, 8000, 16)), chunk(b'data', samples(16, False, 10, 1))]
	write('Bad/empty.wav', b'')
	write('Bad/tooshort.wav', b'RIFF\x04\x00\x00\x00')
	write('Bad/badriff.wav', riff(good, id=b'RIFX'))
	write('Bad/badwave.wav', riff(good, form=b'AVI '))
	write('Bad/nofmt.wav', riff([good[1]]))
	write('Bad/nodata.wav', riff([good[0]]))
	write('Bad/shortfmt.wav', riff([chunk(b'fmt ', fmt(1, 1, 8000, 16)[0:14]), good[1]]))
	write('Bad/fmtpastend.wav', riff([chunk(b'fmt ', fmt(1, 1, 8000, 16), size=100)]))
	write('Bad/badbits.wav', riff([chunk(b'fmt ', fmt(1, 1, 8000, 12, 2)), good[1]]))
	write('Bad/badblockalign.wav', riff([chunk(b'fmt ', fmt(1, 2, 8000, 16, 3)), good[1]]))
	write('Bad/zerochannels.wav', riff([chunk(b'fmt ', fmt(1, 0, 8000, 16, 0)), good[1]]))
	write('Bad/zerorate.wav', riff([chunk(b'fmt ', fmt(1, 1, 0, 16)), good[1]]))
	write('Bad/adpcm.wav', riff([chunk(b'fmt ', fmt(2, 1, 8000, 4, 1) + struct.pack('<H', 0)), good[1]]))
	write('Bad/float16.wav', riff([chunk(b'fmt ', fmt(3, 1, 8000, 16)), good[1]]))
	write('Bad/badguid.wav', riff([chunk(b'fmt ', fmtExtensible(1, 1, 8000, 16, 16, 0x4, bytes(14))), good[1]]))
	write('Bad/shortextensible.wav', riff([chunk(b'fmt ', fmtExtensible(1, 1, 8000, 16, 16, 0x4)[0:24]), good[1]]))
	write('Bad/twofmt.wav', riff([good[0], good[0], good[1]]))


writeGood()
writeBad()
