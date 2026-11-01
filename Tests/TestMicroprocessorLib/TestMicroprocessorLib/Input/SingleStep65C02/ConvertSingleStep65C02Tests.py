# Converts the first tests of each SingleStepTests/65x02 wdc65c02 JSON file into the same
# compact binary as ConvertSingleStepTests.py writes, with "SC02" as its magic.
# https://github.com/SingleStepTests/65x02 (MIT licence).
#
# Usage: python3 ConvertSingleStep65C02Tests.py <download dir> <output dir> <tests per file> [opcode files...]
#
# The 65C02 tests only give each cycle's address, value and whether it was a read or a write.
# The state is written as a W65C816S in emulation mode: S in page 1, D, DBR and PBR 0, E 1.
# Only the read bit of the signals is meaningful.
# Only the start of each JSON file is fetched (HTTP range requests), the 954MB set is never downloaded.
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

URL = 'https://raw.githubusercontent.com/SingleStepTests/65x02/main/wdc65c02/v1/'

def fetch_tests(name, count, download_dir):
    path = os.path.join(download_dir, name + '.part')
    size = 256 * 1024
    while True:
        subprocess.run(['curl', '-s', '-f', '--max-time', '20', '--retry', '10', '-r', '0-%d' % (size - 1), '-o', path, URL + name], check=True)
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
    b = struct.pack('<HHBHHHBHBB', state['pc'], 0x100 | state['s'], state['p'], state['a'], state['x'], state['y'],
                    0, 0, 0, 1)
    b += struct.pack('<H', len(state['ram']))
    for address, value in state['ram']:
        b += struct.pack('<I', address)[:3] + struct.pack('<B', value)
    return b

def pack_cycle(cycle):
    address, value, kind = cycle
    signals = 0x08 if kind == 'read' else 0
    valid = (1 if value is not None else 0) | (2 if address is not None else 0)
    return struct.pack('<I', address if address is not None else 0)[:3] + struct.pack('<BBB', value if value is not None else 0, signals, valid)

def convert(name, count, download_dir, output_dir):
    tests = fetch_tests(name, count, download_dir)
    b = b'SC02' + struct.pack('<HH', 1, len(tests))
    for test in tests:
        b += pack_state(test['initial']) + pack_state(test['final'])
        b += struct.pack('<H', len(test['cycles']))
        for cycle in test['cycles']:
            b += pack_cycle(cycle)
    open(os.path.join(output_dir, name.replace('.json', '.bin')), 'wb').write(b)
    return len(tests), len(b)

if __name__ == '__main__':
    download_dir, output_dir, count = sys.argv[1], sys.argv[2], int(sys.argv[3])
    # WAI (CB) and STP (DB) have no tests.
    names = sys.argv[4:] or ['%02x.json' % op for op in range(256) if op not in (0xCB, 0xDB)]
    total = 0
    for name in names:
        if os.path.exists(os.path.join(output_dir, name.replace('.json', '.bin'))):
            continue
        n, size = convert(name, count, download_dir, output_dir)
        total += size
        print(name, n, size, flush=True)
    print('total', total)
