# Converts the first tests of each SingleStepTests/65816 JSON file into a compact binary.
# https://github.com/SingleStepTests/65816 (MIT licence, Thomas Harte).
#
# Usage: python3 ConvertSingleStepTests.py <download dir> <output dir> <tests per file> [opcode files...]
#
# MVN and MVP keep only a few short copies.
# Only the start of each JSON file is fetched (HTTP range requests), the 2.6GB set is never downloaded.
#
# Binary format, little endian:
#   char[4] "S816", uint16 version (1), uint16 test count
#   per test:
#     initial state, final state:
#       uint16 pc, uint16 s, uint8 p, uint16 a, uint16 x, uint16 y, uint8 dbr, uint16 d, uint8 pbr, uint8 e
#       uint16 ram count, then per entry: uint24 address, uint8 value
#     uint16 cycle count, then per cycle: uint24 address, uint8 value, uint8 signals, uint8 valid
#       signals: bit 0 VDA, 1 VPA, 2 VPB, 3 read, 4 E, 5 M, 6 X, 7 MLB (set means active)
#       valid: bit 0 the value is known, bit 1 the address is known (both are null in the JSON when the bus floats)
import json, os, struct, subprocess, sys

URL = 'https://raw.githubusercontent.com/SingleStepTests/65816/main/v1/'

def fetch_tests(name, count, download_dir):
    path = os.path.join(download_dir, name + '.part')
    size = 256 * 1024
    while True:
        subprocess.run(['curl', '-s', '-f', '--max-time', '120', '--retry', '5', '--retry-all-errors', '-r', '0-%d' % (size - 1), '-o', path, URL + name], check=True)
        text = open(path, encoding='ascii').read()
        tests = []
        decoder = json.JSONDecoder()
        pos = text.index('[') + 1
        complete = len(text) < size
        while len(tests) < count:
            while pos < len(text) and text[pos] in ' \r\n\t,':
                pos += 1
            if pos >= len(text) or text[pos] == ']':
                break
            try:
                test, pos = decoder.raw_decode(text, pos)
            except json.JSONDecodeError:
                break
            tests.append(test)
        if len(tests) >= count or complete:
            return tests
        size *= 4

def pack_state(state):
    b = struct.pack('<HHBHHHBHBB', state['pc'], state['s'], state['p'], state['a'], state['x'], state['y'],
                    state['dbr'], state['d'], state['pbr'], state['e'])
    b += struct.pack('<H', len(state['ram']))
    for address, value in state['ram']:
        b += struct.pack('<I', address)[:3] + struct.pack('<B', value)
    return b

def pack_cycle(cycle):
    address, value, outputs = cycle
    signals = 0
    for bit, (char, active) in enumerate(zip(outputs, 'dpvremxl')):
        if bit == 3:
            if char == 'r':
                signals |= 1 << bit
        elif char == active:
            signals |= 1 << bit
    valid = (1 if value is not None else 0) | (2 if address is not None else 0)
    return struct.pack('<I', address if address is not None else 0)[:3] + struct.pack('<BBB', value if value is not None else 0, signals, valid)

BLOCK_MOVES = ('44.', '54.')
BLOCK_MOVE_TESTS = 20
BLOCK_MOVE_MAX_CYCLES = 7 * 64

def convert(name, count, download_dir, output_dir):
    if name.startswith(BLOCK_MOVES):
        # MVN and MVP copy up to 1000 bytes, seven cycles each.  Keep a few short copies.
        tests = [t for t in fetch_tests(name, 1000, download_dir) if len(t['cycles']) <= BLOCK_MOVE_MAX_CYCLES][:BLOCK_MOVE_TESTS]
    else:
        tests = fetch_tests(name, count, download_dir)
    b = b'S816' + struct.pack('<HH', 1, len(tests))
    for test in tests:
        b += pack_state(test['initial']) + pack_state(test['final'])
        b += struct.pack('<H', len(test['cycles']))
        for cycle in test['cycles']:
            b += pack_cycle(cycle)
    open(os.path.join(output_dir, name.replace('.json', '.bin')), 'wb').write(b)
    return len(tests), len(b)

if __name__ == '__main__':
    download_dir, output_dir, count = sys.argv[1], sys.argv[2], int(sys.argv[3])
    names = sys.argv[4:] or ['%02x.%s.json' % (op, m) for op in range(256) for m in 'en']
    total = 0
    for name in names:
        if os.path.exists(os.path.join(output_dir, name.replace('.json', '.bin'))):
            continue
        n, size = convert(name, count, download_dir, output_dir)
        total += size
        print(name, n, size, flush=True)
    print('total', total)
