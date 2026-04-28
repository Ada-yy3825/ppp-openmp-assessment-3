#!/usr/bin/env python3
"""
Fail the workflow if Markdown files or *comments* in C/C++ sources
contain non-English text.

 ▸ .md files         → whole file is checked
 ▸ C/C++ {h,cpp,cc,…}→ only // … and /* … */ comments are checked

New in this version
-------------------
* Fast Unicode filter: flags any character that belongs to major non-Latin
  ranges (Han, Kana, Hangul, Cyrillic, Greek, etc.).
"""

from __future__ import annotations

import re
import sys
from pathlib import Path
from typing import Iterable, Iterator, List

from langdetect import DetectorFactory, detect_langs, LangDetectException

DetectorFactory.seed = 0

# ── Tuneables ──────────────────────────────────────────────────────────────
CODE_EXTS: tuple[str, ...] = ('.cpp', '.cc', '.cxx', '.hpp', '.h')
MIN_TEXT_LENGTH = 120          # short snippets auto-pass
MIN_PROB_EN = 0.80             # required langdetect prob for 'en'

# Regex matching any character in these Unicode blocks:
#   Han, Hiragana, Katakana, Hangul, Cyrillic, Greek, Thai, Arabic, Hebrew
DISALLOWED_CHARS_RE = re.compile(
    r'['
    r'\u3040-\u30ff'          # Japanese Hiragana & Katakana
    r'\u3400-\u4dbf'          # CJK unified ideographs ext. A
    r'\u4e00-\u9fff'          # CJK unified ideographs
    r'\uf900-\ufaff'          # CJK compatibility ideographs
    r'\uac00-\ud7af'          # Hangul Syllables
    r'\u0400-\u052f'          # Cyrillic
    r'\u0370-\u03ff'          # Greek
    r'\u0590-\u05ff'          # Hebrew
    r'\u0600-\u06ff'          # Arabic
    r'\u0e00-\u0e7f'          # Thai
    r']',
    flags=re.UNICODE,
)

# ── Helpers ────────────────────────────────────────────────────────────────
COMMENT_RE = re.compile(
    r'''
    (?: // (?P<sl>[^\n\r]*?) (?:\r?\n|\Z)      # // single-line
     | /\* (?P<ml>.*?) \*/ )                  # /* multi-line */
    ''',
    re.DOTALL | re.VERBOSE,
)


def iter_comment_blocks(code: str) -> Iterator[str]:
    for m in COMMENT_RE.finditer(code):
        body = m.group('sl') or m.group('ml') or ''
        if body:
            yield body.strip()


def extract_relevant_text(path: Path) -> str:
    try:
        content = path.read_text(encoding='utf-8', errors='ignore')
    except Exception:
        return ''

    if path.suffix.lower() == '.md':
        return content
    return '\n'.join(iter_comment_blocks(content))


def contains_disallowed_chars(text: str) -> bool:
    """Return True if *text* has any character in non-Latin blocks."""
    return bool(DISALLOWED_CHARS_RE.search(text))


def is_probably_english(text: str) -> bool:
    text = text.strip()
    if len(text) < MIN_TEXT_LENGTH:
        return True
    try:
        for lang in detect_langs(text):
            if lang.lang == 'en' and lang.prob >= MIN_PROB_EN:
                return True
    except LangDetectException:
        pass
    return False


# ── Main ───────────────────────────────────────────────────────────────────
def main() -> None:
    root = Path('.')
    offenders: List[str] = []

    patterns: Iterable[str] = ('*.md',) + tuple(f'*{ext}' for ext in CODE_EXTS)

    for pat in patterns:
        for file in root.rglob(pat):
            text = extract_relevant_text(file)
            if not text.strip():
                continue

            # 1. quick Unicode-range filter
            if contains_disallowed_chars(text):
                offenders.append(str(file))
                continue

            # 2. probabilistic detector
            if not is_probably_english(text):
                offenders.append(str(file))

    if offenders:
        print('Non-English content detected in:')
        for f in offenders:
            print(f'  {f}')
        print('Language check: fail')
        sys.exit(1)

    print('✅ All checked comments / documents look English: pass')
    sys.exit(0)


if __name__ == '__main__':
    main()
