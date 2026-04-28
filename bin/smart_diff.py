#!/usr/bin/env python3
"""
smart_diff.py: Robust comparison of two text outputs (token-wise with float tolerance).

Usage:
    python smart_diff.py [--rtol RTOL] [--atol ATOL] actual.txt expected.txt
Returns:
    Exit 0 if all tokens match (with float tolerance), 1 otherwise.
"""
import argparse
import re
import sys
from typing import List, Tuple

import numpy as np

# Regex to recognize floating-point numbers
FLOAT_RE = re.compile(r"""
    ^[+-]?              # optional sign
    (?:\d+\.\d*|\.\d+|\d+)  # decimal or integer
    (?:[eE][+-]?\d+)?  # optional exponent
    $""", re.VERBOSE)


def tokenize(text: str) -> List[str]:
    """Split text into tokens on any whitespace."""
    return re.findall(r"\S+", text)


def _tokens_match(slice_a: List[str],
                  expected: List[str],
                  rtol: float,
                  atol: float) -> Tuple[bool, List[str]]:
    """
    Compare two *equal-length* slices token-by-token.
    Extracted from your original function unchanged.
    """
    errors: List[str] = []
    for idx, (a, e) in enumerate(zip(slice_a, expected)):
        if a == e:
            continue
        if FLOAT_RE.match(a) and FLOAT_RE.match(e):
            try:
                af, ef = float(a), float(e)
            except ValueError:
                errors.append(f"[{idx}] Bad floats: {a!r} vs {e!r}")
            else:
                if not np.isclose(af, ef, rtol=rtol, atol=atol):
                    errors.append(f"[{idx}] Float mismatch: {af} vs {ef}")
        else:
            errors.append(f"[{idx}] String mismatch: {a!r} vs {e!r}")
    return (len(errors) == 0), errors


def compare_tokens(
    actual: List[str],
    expected: List[str],
    rtol: float = 1e-05,
    atol: float = 1e-08,
) -> Tuple[bool, List[str]]:
    """
    Compare `actual` to `expected`, **ignoring any leading or trailing noise**.
    Returns ``(all_match, diagnostics)``.
    """
    # Fast bail-out if the expected sequence is longer than the actual one
    if len(actual) < len(expected):
        return False, [f"Actual tokens shorter than expected "
                       f"({len(actual)} vs {len(expected)})"]

    window = len(expected)
    # test prefix, suffix and nothing more; keeps complexity O(n)
    candidates = (
        actual[:window],          # drop possible *trailing* noise
        actual[-window:],         # drop possible *leading* noise
    )

    for slice_a in candidates:
        ok, errs = _tokens_match(slice_a, expected, rtol, atol)
        if ok:
            return True, []       # found a clean match – success!

    # If we get here, neither prefix nor suffix matched.
    # Fall back to a full comparison so the caller gets detailed errors.
    return _tokens_match(actual[:window], expected, rtol, atol)


def compare_files(
    actual_path: str,
    expected_path: str,
    rtol: float,
    atol: float,
) -> Tuple[bool, List[str]]:
    """Read and compare two files by token."""
    with open(actual_path, 'r') as f:
        actual_text = f.read()
    with open(expected_path, 'r') as f:
        expected_text = f.read()

    atoks = tokenize(actual_text)
    etoks = tokenize(expected_text)
    return compare_tokens(atoks, etoks, rtol, atol)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Compare two text files token-wise with float tolerance"
    )
    parser.add_argument(
        '--rtol', type=float, default=1e-6,
        help='Relative tolerance for float comparison'
    )
    parser.add_argument(
        '--atol', type=float, default=1e-8,
        help='Absolute tolerance for float comparison'
    )
    parser.add_argument(
        'actual', help='Path to actual output file'
    )
    parser.add_argument(
        'expected', help='Path to expected output file'
    )
    args = parser.parse_args()

    ok, errors = compare_files(
        args.actual, args.expected, rtol=args.rtol, atol=args.atol
    )
    if ok:
        return 0

    for err in errors:
        print(err)
    return 1


if __name__ == '__main__':
    sys.exit(main())

