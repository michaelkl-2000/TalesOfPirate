"""Сканер: ищет #include, попавшие ВНУТРЬ блока `namespace Corsairs::Common::... { ... }`.

Используется при доменной обёртке Common в namespace: include, оказавшийся внутри
namespace-блока, протаскивает чужой namespace внутрь текущего, что приводит к
паразитным путям вида `Corsairs::Common::Inventory::Corsairs::Common::Mount`
и ошибкам C2872 на ровном месте.

Сканер использует балансировку фигурных скобок и предварительно стирает
строки/комментарии, чтобы не словить ложноположительные срабатывания.
"""

from __future__ import annotations

import os
import re
import sys


def strip_strings_and_comments(src: str) -> str:
    out: list[str] = []
    i = 0
    n = len(src)
    while i < n:
        c = src[i]
        # line comment
        if c == '/' and i + 1 < n and src[i + 1] == '/':
            while i < n and src[i] != '\n':
                out.append(' ')
                i += 1
            continue
        # block comment
        if c == '/' and i + 1 < n and src[i + 1] == '*':
            out.append(' ')
            out.append(' ')
            i += 2
            while i + 1 < n and not (src[i] == '*' and src[i + 1] == '/'):
                out.append('\n' if src[i] == '\n' else ' ')
                i += 1
            i = min(n, i + 2)
            out.append(' ')
            out.append(' ')
            continue
        # string / char literal
        if c == '"' or c == "'":
            q = c
            out.append(' ')
            i += 1
            while i < n and src[i] != q:
                if src[i] == '\\' and i + 1 < n:
                    out.append(' ')
                    out.append(' ')
                    i += 2
                else:
                    out.append('\n' if src[i] == '\n' else ' ')
                    i += 1
            if i < n:
                out.append(' ')
                i += 1
            continue
        out.append(c)
        i += 1
    return ''.join(out)


def scan(root: str) -> list[tuple[str, int, int]]:
    problems: list[tuple[str, int, int]] = []
    ns_open_re = re.compile(r'^[ \t]*namespace\s+Corsairs::Common\b[^{]*\{', re.MULTILINE)
    include_re = re.compile(r'^[ \t]*#\s*include\b', re.MULTILINE)

    for sub in ('include', 'src'):
        base = os.path.join(root, sub)
        if not os.path.isdir(base):
            continue
        for d, _, files in os.walk(base):
            for f in files:
                if not f.endswith(('.h', '.cpp', '.hpp', '.inl')):
                    continue
                path = os.path.join(d, f)
                with open(path, 'rb') as fp:
                    raw = fp.read()
                if raw.startswith(b'\xef\xbb\xbf'):
                    raw = raw[3:]
                try:
                    text = raw.decode('utf-8')
                except UnicodeDecodeError:
                    text = raw.decode('cp1251', errors='replace')
                clean = strip_strings_and_comments(text)

                for m in ns_open_re.finditer(clean):
                    opener_end = m.end()
                    depth = 1
                    i = opener_end
                    n = len(clean)
                    while i < n and depth > 0:
                        ch = clean[i]
                        if ch == '{':
                            depth += 1
                        elif ch == '}':
                            depth -= 1
                        i += 1
                    closer = i
                    block = clean[opener_end:closer]
                    for inc in include_re.finditer(block):
                        inc_abs = opener_end + inc.start()
                        line_no = clean.count('\n', 0, inc_abs) + 1
                        ns_line = clean.count('\n', 0, m.start()) + 1
                        rel = os.path.relpath(path, root)
                        problems.append((rel, line_no, ns_line))
    return problems


if __name__ == '__main__':
    root = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
        'sources', 'Libraries', 'Common',
    )
    problems = scan(root)
    if not problems:
        print('OK: no #include found inside any namespace Corsairs::Common {...} block')
        sys.exit(0)
    print(f'Found {len(problems)} include(s) inside namespace blocks:')
    for p, il, nl in problems:
        print(f'  {p}: #include at line {il} (ns opens at {nl})')
