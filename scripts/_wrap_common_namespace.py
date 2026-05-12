"""
Wrap each .h/.cpp file in sources/Libraries/common/{include,src}/<Domain>/
into `namespace Corsairs::Common::<Domain> { ... }`.

Rules:
- Skip files already containing `namespace Corsairs::Common` (idempotent).
- Skip third-party (Discord/*) and PCH (stdafx.h/cpp).
- Insert open brace AFTER:
  * leading comment block
  * `#pragma once`
  * the include guard `#ifndef GUARD / #define GUARD` (keep braces inside)
  * all preprocessor lines that look like #include / #define for header constants
  * (we stop at first non-preprocessor, non-comment, non-blank line)
- Insert close brace BEFORE the trailing `#endif` if the file uses an include
  guard; otherwise append at end of file.
- Preserve byte-for-byte unchanged content otherwise (no reformatting).
"""

from pathlib import Path
import re
import sys

ROOT = Path("D:/Projects/MMORPG/TalesOfPirate/sources/Libraries/common")
SKIP_FILES = {"stdafx.h", "stdafx.cpp"}
SKIP_DOMAINS = {"Discord"}  # third-party

DOMAINS = [
    "Database", "Core", "Crypto", "Network",
    "Character", "Item", "Inventory", "Skill", "NPC",
    "World", "Audio", "Effect", "Mount", "Progression",
    "Localization", "Server", "Misc",
]

NS_MARKER = "namespace Corsairs::Common"


def find_open_position(lines: list[str], domain: str) -> tuple[int, str | None]:
    """Return (insert_index, guard_name_or_None). insert_index is the line
    index AFTER which to insert the namespace open."""
    i = 0
    n = len(lines)
    guard_name = None

    # Skip leading comments and blanks
    in_block_comment = False
    while i < n:
        line = lines[i].rstrip()
        stripped = line.strip()
        if in_block_comment:
            if "*/" in line:
                in_block_comment = False
            i += 1
            continue
        if stripped.startswith("/*"):
            if "*/" not in line:
                in_block_comment = True
            i += 1
            continue
        if stripped.startswith("//") or stripped == "":
            i += 1
            continue
        break

    # Now i points at first non-comment/non-blank line.
    # Handle include guard: #ifndef X / #define X
    if i < n - 1:
        m1 = re.match(r"\s*#\s*ifndef\s+(\w+)\s*$", lines[i])
        if m1:
            m2 = re.match(r"\s*#\s*define\s+(\w+)\s*$", lines[i + 1])
            if m2 and m1.group(1) == m2.group(1):
                guard_name = m1.group(1)
                i += 2  # consume guard

    # Now skip leading preprocessor (#include, #pragma, #define, #if/#endif blocks)
    # and blank/comment lines, up to first declaration/definition.
    while i < n:
        line = lines[i]
        stripped = line.strip()
        if stripped == "":
            i += 1
            continue
        if stripped.startswith("//"):
            i += 1
            continue
        if stripped.startswith("/*"):
            # block comment
            while i < n and "*/" not in lines[i]:
                i += 1
            i += 1
            continue
        if stripped.startswith("#"):
            # Preprocessor directive (include, define, pragma, if, etc.)
            i += 1
            # Skip multi-line #define ending with backslash
            while i > 0 and lines[i - 1].rstrip().endswith("\\") and i < n:
                i += 1
            continue
        break

    return i, guard_name


def find_close_position(lines: list[str], guard_name: str | None) -> int:
    """Return index BEFORE which to insert the closing brace."""
    n = len(lines)
    if guard_name is None:
        return n

    # Find last #endif from bottom
    for i in range(n - 1, -1, -1):
        stripped = lines[i].strip()
        if not stripped:
            continue
        if re.match(r"#\s*endif\b", stripped):
            return i
        # If we hit a non-blank, non-#endif line, no guard close — give up
        # and just append at end.
        if stripped.startswith("//") or stripped.startswith("/*"):
            continue
        return n
    return n


def wrap_file(path: Path, domain: str) -> bool:
    # Read raw bytes to detect/preserve UTF-8 BOM
    raw = path.read_bytes()
    has_bom = raw.startswith(b"\xef\xbb\xbf")
    text_bytes = raw[3:] if has_bom else raw
    text = text_bytes.decode("utf-8", errors="strict")
    if NS_MARKER in text:
        return False  # already wrapped

    lines = text.splitlines(keepends=True)
    if not lines:
        return False

    open_idx, guard = find_open_position(lines, domain)
    close_idx = find_close_position(lines, guard)

    if open_idx >= close_idx:
        # Nothing to wrap (empty file body)
        return False

    ns_open = f"\nnamespace Corsairs::Common::{domain} {{\n\n"
    ns_close = f"\n}} // namespace Corsairs::Common::{domain}\n\n"

    new_lines = (
        lines[:open_idx]
        + [ns_open]
        + lines[open_idx:close_idx]
        + [ns_close]
        + lines[close_idx:]
    )
    out_bytes = "".join(new_lines).encode("utf-8")
    if has_bom:
        out_bytes = b"\xef\xbb\xbf" + out_bytes
    path.write_bytes(out_bytes)
    return True


def main():
    only_domain = sys.argv[1] if len(sys.argv) > 1 else None
    total = 0
    skipped = 0
    for kind in ("include", "src"):
        for domain in DOMAINS:
            if only_domain and domain != only_domain:
                continue
            if domain in SKIP_DOMAINS:
                continue
            d = ROOT / kind / domain
            if not d.is_dir():
                continue
            ext = ".h" if kind == "include" else ".cpp"
            for f in sorted(d.glob(f"*{ext}")):
                if f.name in SKIP_FILES:
                    continue
                if wrap_file(f, domain):
                    total += 1
                else:
                    skipped += 1
    print(f"Wrapped {total} files; skipped {skipped} (already wrapped or empty).")


if __name__ == "__main__":
    main()
