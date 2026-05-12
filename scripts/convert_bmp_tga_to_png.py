"""
Конвертация всех .bmp/.tga в .png рядом с оригиналами.
Оригиналы НЕ удаляются — это отдельный этап миграции.

Поведение:
- Идёт по всему репозиторию (кроме .git).
- Пропускает файл, если рядом уже есть .png с тем же базовым именем.
- BMP/TGA с альфой → RGBA, без альфы → RGB (PNG автоматически).
- TGA RLE/индексные/grayscale — поддерживаются Pillow.
- Ошибки логируются, но не останавливают batch.
- В конце — статистика.
"""

from __future__ import annotations

import argparse
import os
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

from PIL import Image


def collect_files(root: Path) -> list[Path]:
    out: list[Path] = []
    for dirpath, dirnames, filenames in os.walk(root):
        if ".git" in dirnames:
            dirnames.remove(".git")
        for name in filenames:
            ext = name.rsplit(".", 1)[-1].lower() if "." in name else ""
            if ext in ("bmp", "tga"):
                out.append(Path(dirpath) / name)
    return out


def convert_one(src: Path) -> tuple[Path, str]:
    """Возвращает (path, status) — status: 'ok' / 'skip' / 'err:...'."""
    dst = src.with_suffix(".png")
    if dst.exists():
        return src, "skip"
    try:
        with Image.open(src) as im:
            mode = im.mode
            # Для P (палитра) сохраняем как RGBA, чтобы прозрачность не потерять;
            # для L (grayscale) — оставляем L, PNG это умеет.
            if mode == "P":
                im = im.convert("RGBA")
            elif mode in ("RGB", "RGBA", "L", "LA"):
                pass
            else:
                im = im.convert("RGBA")
            im.save(dst, format="PNG", optimize=False, compress_level=6)
        return src, "ok"
    except Exception as e:  # noqa: BLE001
        return src, f"err:{type(e).__name__}: {e}"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=".", help="корень обхода")
    parser.add_argument("--workers", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--limit", type=int, default=0, help="0 = без лимита")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    print(f"[scan] root: {root}", flush=True)
    files = collect_files(root)
    if args.limit > 0:
        files = files[: args.limit]
    print(f"[scan] found: {len(files)} files (bmp+tga)", flush=True)

    t0 = time.monotonic()
    ok = 0
    skip = 0
    errs: list[tuple[Path, str]] = []

    with ProcessPoolExecutor(max_workers=args.workers) as pool:
        futures = {pool.submit(convert_one, p): p for p in files}
        done = 0
        for fut in as_completed(futures):
            src, status = fut.result()
            done += 1
            if status == "ok":
                ok += 1
            elif status == "skip":
                skip += 1
            else:
                errs.append((src, status))
            if done % 500 == 0:
                el = time.monotonic() - t0
                print(
                    f"[prog] {done}/{len(files)}  ok={ok} skip={skip} err={len(errs)}  ({el:.1f}s)",
                    flush=True,
                )

    el = time.monotonic() - t0
    print("", flush=True)
    print(f"[done] {len(files)} processed in {el:.1f}s", flush=True)
    print(f"[done]   ok    = {ok}", flush=True)
    print(f"[done]   skip  = {skip}  (png already exists)", flush=True)
    print(f"[done]   err   = {len(errs)}", flush=True)
    if errs:
        print("", flush=True)
        print("[err] first 30:", flush=True)
        for src, msg in errs[:30]:
            print(f"  {src}  ->  {msg}", flush=True)
    return 0 if not errs else 2


if __name__ == "__main__":
    sys.exit(main())
