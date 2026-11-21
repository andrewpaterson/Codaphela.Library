#!/usr/bin/env python3
# clauses.py check [Clause05f.txt ...] [-only Name ...]
#     Each case through g++ -std=c++14 -pedantic-errors, -m32 and -m64: says where what g++ gives isn't the case's
#     expected value.  Every file in Clauses if none is named.  Check a case before the compiler is asked to agree with it.
# clauses.py header
#     Writes the cases into ../TestCppCompilerLib/TestCPPConformanceClauseCases.h: each clause's table (gasConformanceClauseN) is replaced,
#     or added in clause order.  A clause new to the test also needs its CONFORMANCE_GROUP line in TestCPPConformance.cpp.
#
# A cases file is Clauses/ClauseNN.txt, or ClauseNNa.txt, ClauseNNb.txt ... for a clause in parts (joined in name order).  A
# case is a line "### Name | expect | reference" and then its source, to the next "###".  expect is the number main
# returns, the same on both targets and on the host (compare with sizeof, don't return one), or REFUSED for a program
# the compiler must give an error for.  reference is the standard's paragraphs and a line saying what is tested: it
# becomes the comment over the case.  @SAME@ is the Same<T, U> templates, @NEW@ a counting heap with the global
# operator new and delete, @INITLIST@ std::initializer_list.
#
# The cases as the compiler does them: TestCPPConformance.cpp, whose lists say which aren't done (lists.py prints and
# writes them).  hostcheck.py and gcccheck.sh check the cases the test exports, the hand written ones too.
import glob, os, re, subprocess, sys, tempfile
HERE = os.path.dirname(os.path.abspath(__file__))
CLAUSES = os.path.join(HERE, 'Clauses')
HEADER = os.path.join(HERE, '..', 'TestCppCompilerLib', 'TestCPPConformanceClauseCases.h')
TITLES = {2: 'lexical conventions [lex]', 3: 'basic concepts [basic]', 4: 'standard conversions [conv]', 5: 'expressions [expr]', 6: 'statements [stmt.stmt]', 7: 'declarations [dcl.dcl]', 8: 'declarators [dcl.decl]', 9: 'classes [class]', 10: 'derived classes [class.derived]', 11: 'member access control [class.access]', 12: 'special member functions [special]', 13: 'overloading [over]', 14: 'templates [temp]', 15: 'exception handling [except]', 16: 'preprocessing directives [cpp]'}
NEW = """typedef decltype(sizeof(0)) size_t;
char gacHeap[1024];
size_t guiHeap = 0;
size_t guiLastSize = 0;
int giNews = 0;
int giDeletes = 0;
void* Allocate(size_t n) { void* p = gacHeap + guiHeap; for (size_t i = 0; i < n; i++) gacHeap[guiHeap + i] = 0x55; guiHeap += (n + 7) & ~(size_t)7; guiLastSize = n; giNews++; return p; }
void* operator new(size_t n) { return Allocate(n); }
void* operator new[](size_t n) { return Allocate(n); }
void operator delete(void* p) noexcept { if (p) giDeletes++; }
void operator delete[](void* p) noexcept { if (p) giDeletes++; }
void operator delete(void* p, size_t) noexcept { if (p) giDeletes++; }
void operator delete[](void* p, size_t) noexcept { if (p) giDeletes++; }"""
INITLIST = """#ifdef CONFORMANCE_HOST
#include <initializer_list>
#else
typedef decltype(sizeof(0)) size_t;
namespace std { template<class T> class initializer_list { const T* mpFirst; size_t muiSize; public: constexpr initializer_list() noexcept : mpFirst(0), muiSize(0) {} constexpr size_t size() const noexcept { return muiSize; } constexpr const T* begin() const noexcept { return mpFirst; } constexpr const T* end() const noexcept { return mpFirst + muiSize; } }; }
#endif"""
SAME = "template<class T, class U> struct Same { enum { V = 0 }; };\ntemplate<class T> struct Same<T, T> { enum { V = 1 }; };"


def read_text(text):
    cases = []; cur = None
    for line in text.split('\n'):
        if line.startswith('### '):
            name, expect, ref = [p.strip() for p in line[4:].split('|', 2)]
            cur = {'name': name, 'expect': expect, 'ref': ref, 'src': []}
            cases.append(cur)
        elif cur is not None:
            cur['src'].append(line)
    for c in cases:
        while c['src'] and not c['src'][-1].strip(): c['src'].pop()
        c['source'] = '\n'.join(c['src']).replace('@SAME@', SAME).replace('@NEW@', NEW).replace('@INITLIST@', INITLIST) + '\n'
    names = [c['name'] for c in cases]
    assert len(names) == len(set(names)), [n for n in names if names.count(n) > 1]
    return cases


def clauses():
    """Clause number -> its files, in name order."""
    found = {}
    for path in sorted(glob.glob(os.path.join(CLAUSES, 'Clause[0-9][0-9]*.txt'))):
        found.setdefault(int(re.match(r'Clause(\d+)', os.path.basename(path)).group(1)), []).append(path)
    return found


def host(c, tmp):
    out = []
    for m in ('-m32', '-m64'):
        src = os.path.join(tmp, 'c.cpp'); exe = os.path.join(tmp, 'c')
        open(src, 'w').write('#define CONFORMANCE_HOST 1\n#include <stdio.h>\n#define main ConformanceMain\n#line 1\n' + c['source'] + '#undef main\nint main() { printf("%d\\n", ConformanceMain()); return 0; }\n')
        r = subprocess.run(['g++', '-std=c++14', '-pedantic-errors', m, '-msse2', '-mfpmath=sse', '-o', exe, src], capture_output=True, text=True, stdin=subprocess.DEVNULL)
        if r.returncode != 0:
            out.append(('REFUSED', ([l for l in r.stderr.splitlines() if 'error' in l] or [''])[0][:130]))
            continue
        try:
            r = subprocess.run([exe], capture_output=True, text=True, stdin=subprocess.DEVNULL, timeout=10)
            out.append((r.stdout.strip() or ('exit %d' % r.returncode), ''))
        except subprocess.TimeoutExpired:
            out.append(('TIMEOUT', ''))
    return out


def table(cases, group):
    lines = ['static const SConformanceCase gasConformance%s[] =\n{' % group]
    lastref = None
    for c in cases:
        if c['ref'] != lastref:
            lines.append('\t//%s' % c['ref']); lastref = c['ref']
        lines.append('\t{')
        lines.append('\t\t"%s", %s,' % (c['name'], 'CONFORMANCE_REFUSED' if c['expect'] == 'REFUSED' else c['expect']))
        for l in c['source'].rstrip('\n').split('\n'):
            #(?\? where the source has ??, so the C++ the test is built from has no trigraph of its own.)
            lines.append('\t\t"%s\\n"' % re.sub(r'\?(?=\?)', r'?\\', l.replace('\\', '\\\\').replace('"', '\\"')))
        lines.append('\t},')
    lines.append('};')
    return '\n'.join(lines) + '\n'


def check(arguments):
    only = arguments[arguments.index('-only') + 1:] if '-only' in arguments else []
    files = arguments[:arguments.index('-only')] if '-only' in arguments else arguments
    files = [os.path.join(CLAUSES, f) for f in files] or [p for paths in clauses().values() for p in paths]
    with tempfile.TemporaryDirectory() as tmp:
        for path in files:
            cases = read_text(open(path).read()); wrong = 0
            for c in cases:
                if only and c['name'] not in only: continue
                res = host(c, tmp)
                if any(v != c['expect'] for v, _ in res):
                    wrong += 1
                    print('%-34s expect %-8s g++ -m32 %s  -m64 %s  %s' % (c['name'], c['expect'], res[0][0], res[1][0], res[0][1] or res[1][1]))
            print('%s: %d cases, %d differ' % (os.path.basename(path), len(cases), wrong))


def header():
    s = open(HEADER).read()
    for n, paths in sorted(clauses().items()):
        new = table(read_text('\n'.join(open(p).read() for p in paths)), 'Clause%d' % n)
        m = re.search(r'static const SConformanceCase gasConformanceClause%d\[\] =\n\{.*?\n\};\n' % n, s, re.S)
        if m:
            s = s[:m.start()] + new + s[m.end():]
            continue
        # A clause's first cases: its table goes before the next clause's, or last.
        later = [(int(x.group(1)), x.start()) for x in re.finditer(r'\n\n//Clause (\d+), ', s) if int(x.group(1)) > n]
        block = '\n\n//Clause %d, %s.\n%s' % (n, TITLES[n], new)
        if later:
            at = min(later)[1]
            s = s[:at] + block.rstrip('\n') + '\n' + s[at:]
        else:
            s = s.rstrip('\n') + '\n' + block
    assert all(ord(c) < 128 for c in s)
    open(HEADER, 'w', newline='\n').write(s)
    print('written', os.path.normpath(HEADER))


if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == 'check':
        check(sys.argv[2:])
    elif len(sys.argv) > 1 and sys.argv[1] == 'header':
        header()
    else:
        sys.exit('clauses.py check [files] [-only names] | header')
