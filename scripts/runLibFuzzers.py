#!/usr/bin/env python3
# Copyright (c) 2026 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

"""Cross-platform libFuzzer runner, corpus manager, and crash reporter for WarfLang.

Extracts built libFuzzer artifacts (or locates fuzzer binaries in a directory),
splits a wall-clock fuzzing budget (default: 180 seconds / 3 minutes) evenly
across all discovered `*Fuzzer` executables, seeds them from the persistent
corpus and WarfLang test suites, collects crash/timeout/OOM reproducers, and
writes a `summary.json` compatible with `scripts/geneticFuzzer.py
--file-github-issues-from`.
"""

import argparse
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import time
from typing import Dict, List, Optional, Sequence, Set, Tuple
import zipfile

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from geneticFuzzer import (  # noqa: E402
    BUILTIN_TEMPLATES,
    file_github_issues_for_findings,
    finding_fingerprint,
    load_seeds_from_directory,
    strip_fuzzer_and_lit_headers,
)

BUILTIN_IR_SEEDS = [
    "func @main() {\nentry:\n  %0 = const 42\n  ret %0\n}\n",
    "func @add(%a, %b) {\nentry:\n  %0 = add %a, %b\n  ret %0\n}\n"
    "func @main() {\nentry:\n  %1 = call @add(10, 20)\n  ret %1\n}\n",
    "func @branch(%x) {\nentry:\n  %c = lt %x, 10\n  br_cond %c, then_bb, else_bb\n"
    "then_bb:\n  %t = add %x, 1\n  jmp merge_bb\n"
    "else_bb:\n  %e = sub %x, 1\n  jmp merge_bb\n"
    "merge_bb:\n  ret %x\n}\n",
]

IGNORED_EXTENSIONS = {
    ".zip",
    ".pdb",
    ".ilk",
    ".lib",
    ".a",
    ".so",
    ".dylib",
    ".dll",
    ".cpp",
    ".h",
    ".o",
    ".obj",
    ".d",
    ".json",
    ".txt",
    ".md",
}

ASAN_SUMMARY_RE = re.compile(
    r"SUMMARY:\s+\w+Sanitizer:\s+([^\s(]+)(?:\s+\([^)]*\))?(?:\s+in\s+([^\r\n]+))?"
)
ASAN_ERROR_RE = re.compile(r"ERROR:\s+(AddressSanitizer|UndefinedBehaviorSanitizer):\s+([^\s:]+)")
LIBFUZZER_ERROR_RE = re.compile(r"ERROR:\s+libFuzzer:\s+([^\r\n(]+)")
STACK_FRAME_RE = re.compile(r"^\s*#\d+\s+0x[0-9a-fA-F]+\s+in\s+([^\r\n]+)")
RAW_STACK_LINE_RE = re.compile(r"^\s*#\d+\s+0x[0-9a-fA-F]+(?:\s+in\s+|\s+\()[^\r\n]+")
CRASH_BANNER_RE = re.compile(
    r"(?:==\d+==(?:ERROR|WARNING):|UndefinedBehaviorSanitizer:|AddressSanitizer:|"
    r"Assertion failed|runtime error:|ALARM: working on the last Unit)"
)
STACK_SECTION_HEADER_RE = re.compile(
    r"^(?:==\d+==(?:ERROR|WARNING):.*|"
    r"(?:freed|previously allocated|allocated) by thread .*|"
    r"Address 0x[0-9a-fA-F]+ is .*|"
    r"SUMMARY:\s+\w+Sanitizer:.*|"
    r".*runtime error:.*|"
    r"Assertion failed:.*)$"
)
RUNNING_INPUT_RE = re.compile(r"^Running:\s+(.+)$", re.MULTILINE)

IGNORED_FRAME_PREFIXES = (
    "__asan",
    "__ubsan",
    "__sanitizer",
    "__scrt",
    "fuzzer::",
    "LLVMFuzzerTestOneInput",
    "std::",
    "operator new",
    "operator delete",
    "malloc",
    "free",
    "abort",
    "raise",
    "gsignal",
    "_sigtramp",
    " BaseThreadInitThunk",
    "RtlUserThreadStart",
    "main",
    "start",
)


@dataclass
class LibFuzzerFinding:
    fuzzer: str
    kind: str  # "crash", "hang", "oom"
    description: str
    program: str
    details: str
    fingerprint: str
    call_stack: str = ""
    artifact_path: Optional[Path] = None


def extract_zip_archives(artifact_dir: Path) -> Path:
    """Extract any `.zip` archives found under `artifact_dir` and return search root."""
    zip_files = sorted(artifact_dir.rglob("*.zip"))
    if not zip_files:
        return artifact_dir

    extract_root = artifact_dir / "extracted"
    extract_root.mkdir(parents=True, exist_ok=True)
    for zpath in zip_files:
        try:
            with zipfile.ZipFile(zpath, "r") as zf:
                zf.extractall(extract_root)
            print(f"Extracted archive: {zpath} -> {extract_root}")
        except zipfile.BadZipFile as exc:
            print(f"Warning: failed to extract {zpath}: {exc}", file=sys.stderr)
    return extract_root


def discover_fuzzer_binaries(search_root: Path) -> List[Path]:
    """Find all `*Fuzzer` / `*Fuzzer.exe` executables under `search_root`."""
    if not search_root.exists():
        return []

    candidates: Dict[str, Path] = {}
    for path in sorted(search_root.rglob("*")):
        if not path.is_file():
            continue
        if any(part.lower().endswith(".dsym") for part in path.parts):
            continue
        if path.suffix.lower() in IGNORED_EXTENSIONS:
            continue
        stem = path.stem if path.suffix.lower() == ".exe" else path.name
        if not stem.endswith("Fuzzer"):
            continue
        if platform.system() != "Windows":
            try:
                current_mode = path.stat().st_mode
                path.chmod(current_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
            except OSError:
                pass
        candidates[stem] = path

    return [candidates[name] for name in sorted(candidates.keys())]


def load_wir_seeds(directory: Path) -> List[str]:
    """Load `.wir` IR test files from `directory`, stripping lit RUN/CHECK lines."""
    seeds: List[str] = []
    if not directory.exists():
        return seeds
    for wf in sorted(directory.rglob("*.wir")):
        try:
            raw = wf.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        cleaned = strip_fuzzer_and_lit_headers(raw)
        if cleaned:
            seeds.append(cleaned)
    return seeds


def populate_seed_directory(
    fuzzer_name: str,
    seed_dirs: Sequence[Path],
    dest_dir: Path,
) -> int:
    """Write deduplicated initial seed files for `fuzzer_name` into `dest_dir`."""
    dest_dir.mkdir(parents=True, exist_ok=True)
    seeds: List[str] = []

    if "ir" in fuzzer_name.lower():
        for sdir in seed_dirs:
            seeds.extend(load_wir_seeds(sdir))
        seeds.extend(BUILTIN_IR_SEEDS)
    else:
        for sdir in seed_dirs:
            seeds.extend(load_seeds_from_directory(sdir))
        seeds.extend(BUILTIN_TEMPLATES)

    written = 0
    seen_hashes: Set[str] = set()
    for content in seeds:
        data = content.encode("utf-8", errors="ignore")
        if not data:
            continue
        digest = hashlib.sha1(data).hexdigest()
        if digest in seen_hashes:
            continue
        seen_hashes.add(digest)
        (dest_dir / digest).write_bytes(data)
        written += 1
    return written


def normalize_frame_symbol(raw_frame: str) -> str:
    """Normalize a stack frame string to a platform-independent function/file label."""
    # Strip trailing file/module path or address offset
    cleaned = raw_frame.strip()
    # Extract function signature before file path if present
    # e.g. "Parser::ParsePrimaryExpression() /path/to/Parser.cpp:123:45"
    parts = re.split(r"\s+(?=[A-Za-z]:\\|/|\()", cleaned, maxsplit=1)
    func_name = parts[0].strip()
    # Strip template/parameter noise if overly long
    if "(" in func_name:
        func_name = func_name.split("(", 1)[0].strip()
    return func_name


def extract_crash_metadata(
    fuzzer_name: str,
    output_text: str,
    artifact_name: str = "",
) -> Tuple[str, str]:
    """Return `(kind, normalized_description)` from libFuzzer / ASan output."""
    kind = "crash"
    lower_art = artifact_name.lower()
    if lower_art.startswith("timeout-") or "ERROR: libFuzzer: timeout" in output_text:
        kind = "hang"
    elif lower_art.startswith("oom-") or "ERROR: libFuzzer: out-of-memory" in output_text:
        kind = "oom"

    reason = "crash"
    m_asan_err = ASAN_ERROR_RE.search(output_text)
    m_summary = ASAN_SUMMARY_RE.search(output_text)
    m_lf_err = LIBFUZZER_ERROR_RE.search(output_text)

    if m_asan_err:
        reason = f"{m_asan_err.group(1)}: {m_asan_err.group(2)}"
    elif m_summary:
        reason = f"Sanitizer: {m_summary.group(1)}"
    elif m_lf_err:
        lf_reason = m_lf_err.group(1).strip()
        # Normalize variable timeout/oom numbers (e.g. "timeout after 11 seconds")
        lf_reason = re.sub(r"\b\d+\s*(seconds|Mb|MB|bytes)\b", "", lf_reason).strip()
        reason = f"libFuzzer: {lf_reason}"

    top_frame = ""
    for line in output_text.splitlines():
        m_frame = STACK_FRAME_RE.match(line)
        if not m_frame:
            continue
        raw_sym = m_frame.group(1).strip()
        if any(raw_sym.startswith(prefix) for prefix in IGNORED_FRAME_PREFIXES):
            continue
        if f"{fuzzer_name}.cpp" in raw_sym or "FuzzTest" in raw_sym:
            # Prefer the underlying WarfCore frame if one exists, but keep as fallback
            if not top_frame:
                top_frame = normalize_frame_symbol(raw_sym)
            continue
        top_frame = normalize_frame_symbol(raw_sym)
        break

    if not top_frame and m_summary and m_summary.group(2):
        candidate = normalize_frame_symbol(m_summary.group(2))
        if not any(candidate.startswith(p) for p in IGNORED_FRAME_PREFIXES):
            top_frame = candidate

    if top_frame:
        description = f"{fuzzer_name}: {reason} in {top_frame}"
    else:
        description = f"{fuzzer_name}: {reason}"

    return kind, description


def format_crashing_input(data: bytes) -> Tuple[str, str]:
    """Return `(program_str, hex_preview)` for a raw crashing input."""
    hex_preview = data[:512].hex()
    try:
        decoded = data.decode("utf-8")
        # If printable enough, use directly (escaping triple backticks)
        printable_ratio = sum(1 for ch in decoded if ch.isprintable() or ch in "\n\r\t") / max(1, len(decoded))
        if printable_ratio >= 0.80:
            return decoded.replace("```", "` ` `"), hex_preview
    except UnicodeDecodeError:
        pass
    escaped = data.decode("utf-8", errors="backslashreplace").replace("```", "` ` `")
    return escaped, hex_preview


def extract_call_stack(output_text: str) -> str:
    """Extract formatted sanitizer/libFuzzer call stack lines from `output_text`."""
    if not output_text:
        return ""
    collected: List[str] = []
    pending_header: Optional[str] = None
    has_frames = False

    for line in output_text.splitlines():
        stripped = line.strip()
        if RAW_STACK_LINE_RE.match(line):
            if pending_header is not None:
                collected.append(pending_header)
                pending_header = None
            collected.append(f"    {stripped}")
            has_frames = True
        elif STACK_SECTION_HEADER_RE.match(stripped):
            if stripped.startswith("SUMMARY:"):
                collected.append(stripped)
            else:
                pending_header = stripped

    if not has_frames:
        return ""
    return "\n".join(collected)


def extract_crash_report(output_text: str, max_lines: int = 160) -> str:
    """Extract the relevant crash report section from libFuzzer/sanitizer output."""
    lines = output_text.strip().splitlines()
    if not lines:
        return ""
    for idx, line in enumerate(lines):
        if CRASH_BANNER_RE.search(line):
            start = max(0, idx - 2)
            return "\n".join(lines[start : start + max_lines])
    return "\n".join(lines[-max_lines:])


def is_startup_or_loader_failure(returncode: int, output_text: str) -> bool:
    """Return True if a fuzzer process failed at startup (e.g. missing DLL/shared lib) without running inputs."""
    # Windows STATUS_DLL_NOT_FOUND (0xC0000135 = 3221225781 / -1073741515)
    if returncode in (3221225781, -1073741515, 126, 127):
        return True
    lower_out = output_text.lower()
    loader_markers = (
        "error while loading shared libraries",
        "dyld: library not loaded",
        "dyld[",
        "library not loaded:",
        "status_dll_not_found",
    )
    if any(m in lower_out for m in loader_markers):
        return True
    # If there is no output at all and no stack trace, the process died before libFuzzer initialized
    if not output_text.strip():
        return True
    return False


def replay_crash_artifact(
    fuzzer_bin: Path,
    artifact_path: Path,
    env: Dict[str, str],
    timeout: int = 15,
) -> str:
    """Re-run `fuzzer_bin` on a single `artifact_path` to capture a dedicated symbolized call stack."""
    cmd = [
        str(fuzzer_bin.resolve()),
        f"-timeout={max(5, timeout - 5)}",
        str(artifact_path.resolve()),
    ]
    try:
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            errors="replace",
            env=env,
            timeout=timeout,
        )
        return proc.stdout or ""
    except subprocess.TimeoutExpired as exc:
        raw_out = exc.stdout or b""
        if isinstance(raw_out, bytes):
            return raw_out.decode("utf-8", errors="replace")
        return raw_out
    except OSError:
        return ""


def build_fuzzer_env(fuzzer_bin: Path) -> Dict[str, str]:
    """Construct environment variables for running a libFuzzer binary."""
    env = os.environ.copy()
    # Disable LeakSanitizer by default so platform differences don't cause false-positive exits
    existing_asan = env.get("ASAN_OPTIONS", "")
    asan_opts = ["detect_leaks=0", "symbolize=1", "print_stacktrace=1"]
    if existing_asan:
        asan_opts.append(existing_asan)
    env["ASAN_OPTIONS"] = ":".join(asan_opts)

    existing_ubsan = env.get("UBSAN_OPTIONS", "")
    ubsan_opts = ["print_stacktrace=1", "symbolize=1"]
    if existing_ubsan:
        ubsan_opts.append(existing_ubsan)
    env["UBSAN_OPTIONS"] = ":".join(ubsan_opts)

    bin_dir = str(fuzzer_bin.parent.resolve())
    if platform.system() == "Darwin":
        dyld = env.get("DYLD_LIBRARY_PATH", "")
        env["DYLD_LIBRARY_PATH"] = f"{bin_dir}:{dyld}" if dyld else bin_dir
    elif platform.system() == "Linux":
        ld = env.get("LD_LIBRARY_PATH", "")
        env["LD_LIBRARY_PATH"] = f"{bin_dir}:{ld}" if ld else bin_dir
    elif platform.system() == "Windows":
        path_val = env.get("PATH", "")
        llvm_win_root = Path(r"C:\Program Files\LLVM")
        llvm_win_bin = llvm_win_root / "bin"
        extra_paths = [bin_dir]
        if llvm_win_bin.is_dir():
            extra_paths.append(str(llvm_win_bin))
        # Also include Clang's Windows sanitizer runtime dir (for clang_rt.asan_dynamic-x86_64.dll)
        clang_lib_root = llvm_win_root / "lib" / "clang"
        if clang_lib_root.is_dir():
            for win_rt_dir in sorted(clang_lib_root.glob("*/lib/windows"), reverse=True):
                if win_rt_dir.is_dir():
                    extra_paths.append(str(win_rt_dir))
        env["PATH"] = ";".join(extra_paths + ([path_val] if path_val else []))

    symbolizer = shutil.which("llvm-symbolizer", path=env.get("PATH"))
    if symbolizer and "ASAN_SYMBOLIZER_PATH" not in env:
        env["ASAN_SYMBOLIZER_PATH"] = symbolizer

    return env


def run_single_fuzzer(
    fuzzer_bin: Path,
    budget_seconds: int,
    per_input_timeout: int,
    rss_limit_mb: int,
    corpus_dir: Path,
    seed_dir: Path,
    crashes_dir: Path,
    os_name: str,
    max_restarts: int = 3,
) -> Tuple[int, List[LibFuzzerFinding]]:
    """Run a single libFuzzer binary for up to `budget_seconds`, capturing crashes."""
    fuzzer_name = fuzzer_bin.stem if fuzzer_bin.suffix.lower() == ".exe" else fuzzer_bin.name
    corpus_dir.mkdir(parents=True, exist_ok=True)
    crashes_dir.mkdir(parents=True, exist_ok=True)

    env = build_fuzzer_env(fuzzer_bin)
    findings: List[LibFuzzerFinding] = []
    seen_fingerprints: Set[str] = set()
    known_artifacts: Set[Path] = set(crashes_dir.glob(f"{fuzzer_name}-*"))

    start_time = time.monotonic()
    attempt = 0

    while attempt < max_restarts:
        elapsed = time.monotonic() - start_time
        remaining = int(round(budget_seconds - elapsed))
        if remaining <= 0:
            break
        attempt += 1

        artifact_prefix = f"{crashes_dir.resolve()}{os.sep}{fuzzer_name}-"
        cmd = [
            str(fuzzer_bin.resolve()),
            f"-max_total_time={remaining}",
            f"-timeout={per_input_timeout}",
            f"-rss_limit_mb={rss_limit_mb}",
            f"-artifact_prefix={artifact_prefix}",
            "-print_final_stats=1",
            str(corpus_dir.resolve()),
            str(seed_dir.resolve()),
        ]

        print(f"\n=== Running {fuzzer_name} (attempt {attempt}, budget {remaining}s) ===")
        proc_timeout = remaining + max(30, per_input_timeout * 2)
        try:
            proc = subprocess.run(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                errors="replace",
                env=env,
                timeout=proc_timeout,
            )
            output = proc.stdout or ""
            returncode = proc.returncode
        except subprocess.TimeoutExpired as exc:
            raw_out = exc.stdout or b""
            if isinstance(raw_out, bytes):
                output = raw_out.decode("utf-8", errors="replace")
            else:
                output = raw_out
            output += f"\nERROR: libFuzzer: timeout after {proc_timeout} seconds (process wrapper)"
            returncode = 124

        # Print tail of fuzzer output for CI logs
        out_lines = output.strip().splitlines()
        if len(out_lines) > 40:
            print("\n".join(out_lines[-40:]))
        elif out_lines:
            print("\n".join(out_lines))

        current_artifacts = set(crashes_dir.glob(f"{fuzzer_name}-*"))
        new_artifacts = sorted(current_artifacts - known_artifacts)
        known_artifacts = current_artifacts

        if returncode == 0 and not new_artifacts:
            break

        # If a crash occurred while replaying an existing corpus or seed file, remove it
        # so subsequent attempts (and the persistent corpus) are not blocked by it.
        running_matches = RUNNING_INPUT_RE.findall(output)
        if running_matches:
            last_input = Path(running_matches[-1].strip())
            if last_input.exists():
                resolved_parents = last_input.resolve().parents
                if corpus_dir.resolve() in resolved_parents or seed_dir.resolve() in resolved_parents:
                    print(f"Quarantining crashing input from corpus/seed dir: {last_input}")
                    try:
                        last_input.unlink()
                    except OSError:
                        pass

        added_new_finding = False
        if new_artifacts:
            for art_path in new_artifacts:
                try:
                    raw_bytes = art_path.read_bytes()
                except OSError:
                    raw_bytes = b""
                program_str, hex_preview = format_crashing_input(raw_bytes)
                art_output = output
                call_stack = extract_call_stack(art_output)
                if not call_stack or len(new_artifacts) > 1:
                    replayed = replay_crash_artifact(
                        fuzzer_bin, art_path, env, timeout=max(10, per_input_timeout + 5)
                    )
                    if replayed.strip():
                        art_output = replayed
                        call_stack = extract_call_stack(art_output) or call_stack
                kind, description = extract_crash_metadata(fuzzer_name, art_output, art_path.name)
                fp = finding_fingerprint(kind, description)
                if fp in seen_fingerprints:
                    continue
                seen_fingerprints.add(fp)
                added_new_finding = True
                crash_report = extract_crash_report(art_output)
                details = (
                    f"OS: {os_name}\n"
                    f"Fuzzer: {fuzzer_name}\n"
                    f"Exit Code: {returncode}\n"
                    f"Artifact: {art_path.name} ({len(raw_bytes)} bytes)\n"
                    f"Hex (first 512B): {hex_preview}\n\n"
                    f"{crash_report}"
                )
                findings.append(
                    LibFuzzerFinding(
                        fuzzer=fuzzer_name,
                        kind=kind,
                        description=description,
                        program=program_str,
                        details=details,
                        fingerprint=fp,
                        call_stack=call_stack,
                        artifact_path=art_path,
                    )
                )
        elif returncode != 0:
            if is_startup_or_loader_failure(returncode, output):
                print(
                    f"Warning: {fuzzer_name} exited with code {returncode} at startup "
                    f"(missing runtime library/DLL or empty output); skipping issue filing.",
                    file=sys.stderr,
                )
                break
            call_stack = extract_call_stack(output)
            kind, description = extract_crash_metadata(fuzzer_name, output)
            fp = finding_fingerprint(kind, description)
            if fp not in seen_fingerprints:
                seen_fingerprints.add(fp)
                added_new_finding = True
                crash_report = extract_crash_report(output)
                details = (
                    f"OS: {os_name}\n"
                    f"Fuzzer: {fuzzer_name}\n"
                    f"Exit Code: {returncode}\n\n"
                    f"{crash_report}"
                )
                findings.append(
                    LibFuzzerFinding(
                        fuzzer=fuzzer_name,
                        kind=kind,
                        description=description,
                        program="",
                        details=details,
                        fingerprint=fp,
                        call_stack=call_stack,
                    )
                )

        if not added_new_finding:
            break

    corpus_count = sum(1 for p in corpus_dir.iterdir() if p.is_file())
    return corpus_count, findings


def build_arg_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="runLibFuzzers",
        description="Run WarfLang libFuzzer binaries across platforms, manage corpus, and report crashes.",
    )
    p.add_argument(
        "--artifact-dir",
        type=str,
        default="fuzzer-artifact",
        help="Directory containing downloaded libFuzzer zip artifact(s) or binaries (default: fuzzer-artifact).",
    )
    p.add_argument(
        "--corpus-root",
        type=str,
        default="fuzz-corpus/libfuzzer",
        help="Root directory for persistent per-fuzzer corpuses (default: fuzz-corpus/libfuzzer).",
    )
    p.add_argument(
        "--seed-dir",
        type=str,
        default="test/filecheck,fuzz-corpus/corpus",
        help="Comma-separated seed directories (default: test/filecheck,fuzz-corpus/corpus).",
    )
    p.add_argument(
        "--output-dir",
        type=str,
        default="fuzz-output",
        help="Output directory for summary.json, crash artifacts, and corpus snapshot (default: fuzz-output).",
    )
    p.add_argument(
        "--timeout",
        type=int,
        default=180,
        help="Total wall-clock fuzzing time in seconds split across all fuzzers (default: 180).",
    )
    p.add_argument(
        "--per-input-timeout",
        type=int,
        default=10,
        help="Per-input timeout in seconds passed to libFuzzer -timeout (default: 10).",
    )
    p.add_argument(
        "--rss-limit-mb",
        type=int,
        default=2048,
        help="Memory limit in MB passed to libFuzzer -rss_limit_mb (default: 2048).",
    )
    p.add_argument(
        "--os-name",
        type=str,
        default=None,
        help="Optional OS label for reports (defaults to $RUNNER_OS or platform.system()).",
    )
    p.add_argument(
        "--file-github-issues",
        action="store_true",
        default=False,
        help="File deduplicated GitHub issues for any discovered findings via `gh` CLI.",
    )
    p.add_argument(
        "--file-github-issues-from",
        dest="file_github_issues_from",
        type=str,
        default=None,
        help="Standalone mode: read findings from an existing summary.json and file deduplicated GitHub issues.",
    )
    p.add_argument(
        "--repo",
        type=str,
        default=None,
        help="GitHub repository (`owner/name`) for issue filing (defaults to $GITHUB_REPOSITORY).",
    )
    p.add_argument(
        "--run-url",
        type=str,
        default=None,
        help="Optional CI workflow run URL to include in filed GitHub issues.",
    )
    p.add_argument(
        "--dry-run-issues",
        action="store_true",
        default=False,
        help="Print GitHub issue titles/fingerprints without invoking `gh`.",
    )
    return p


def main() -> int:
    parser = build_arg_parser()
    args = parser.parse_args()

    if args.file_github_issues_from:
        summary_path = Path(args.file_github_issues_from)
        if not summary_path.exists():
            print(f"Summary file not found: {summary_path}", file=sys.stderr)
            return 1
        data = json.loads(summary_path.read_text(encoding="utf-8"))
        findings_list = data.get("findings", [])
        file_github_issues_for_findings(
            findings_list,
            repo=args.repo,
            run_url=args.run_url,
            dry_run=args.dry_run_issues,
        )
        return 0

    os_name = args.os_name or os.environ.get("RUNNER_OS") or platform.system()
    artifact_dir = Path(args.artifact_dir)
    search_root = extract_zip_archives(artifact_dir)
    fuzzer_bins = discover_fuzzer_binaries(search_root)

    if not fuzzer_bins:
        print(f"Error: no *Fuzzer binaries found under {artifact_dir}", file=sys.stderr)
        return 1

    fuzzer_names = [
        (b.stem if b.suffix.lower() == ".exe" else b.name) for b in fuzzer_bins
    ]
    per_fuzzer_budget = max(1, int(args.timeout // len(fuzzer_bins)))
    print(
        f"Discovered {len(fuzzer_bins)} libFuzzer targets on {os_name}: "
        f"{', '.join(fuzzer_names)} ({per_fuzzer_budget}s per fuzzer, {args.timeout}s total)"
    )

    corpus_root = Path(args.corpus_root)
    corpus_root.mkdir(parents=True, exist_ok=True)

    output_dir = Path(args.output_dir)
    crashes_dir = output_dir / "crashes"
    findings_dir = output_dir / "findings"
    out_corpus_root = output_dir / "corpus"
    crashes_dir.mkdir(parents=True, exist_ok=True)
    findings_dir.mkdir(parents=True, exist_ok=True)
    out_corpus_root.mkdir(parents=True, exist_ok=True)

    seed_dirs = [
        Path(part.strip())
        for part in args.seed_dir.split(",")
        if part.strip()
    ]

    all_findings: List[LibFuzzerFinding] = []
    corpus_counts: Dict[str, int] = {}

    with tempfile.TemporaryDirectory(prefix="warf_libfuzzer_seeds_") as tmp_seed_root:
        for fuzzer_bin in fuzzer_bins:
            fuzzer_name = fuzzer_bin.stem if fuzzer_bin.suffix.lower() == ".exe" else fuzzer_bin.name
            fuzzer_corpus_dir = corpus_root / fuzzer_name
            fuzzer_seed_dir = Path(tmp_seed_root) / fuzzer_name
            seed_count = populate_seed_directory(fuzzer_name, seed_dirs, fuzzer_seed_dir)
            existing_count = (
                sum(1 for p in fuzzer_corpus_dir.iterdir() if p.is_file())
                if fuzzer_corpus_dir.exists()
                else 0
            )
            print(
                f"[{fuzzer_name}] Loaded {existing_count} persistent corpus files "
                f"and {seed_count} seed inputs."
            )

            final_count, findings = run_single_fuzzer(
                fuzzer_bin=fuzzer_bin,
                budget_seconds=per_fuzzer_budget,
                per_input_timeout=args.per_input_timeout,
                rss_limit_mb=args.rss_limit_mb,
                corpus_dir=fuzzer_corpus_dir,
                seed_dir=fuzzer_seed_dir,
                crashes_dir=crashes_dir,
                os_name=os_name,
            )
            corpus_counts[fuzzer_name] = final_count
            all_findings.extend(findings)

            # Snapshot updated corpus to output_dir/corpus/<fuzzer_name>
            dest_fuzzer_corpus = out_corpus_root / fuzzer_name
            dest_fuzzer_corpus.mkdir(parents=True, exist_ok=True)
            for cfile in fuzzer_corpus_dir.iterdir():
                if cfile.is_file():
                    shutil.copy2(cfile, dest_fuzzer_corpus / cfile.name)

    # Deduplicate findings across fuzzers by fingerprint
    unique_findings: List[LibFuzzerFinding] = []
    seen_fps: Set[str] = set()
    for f in all_findings:
        if f.fingerprint in seen_fps:
            continue
        seen_fps.add(f.fingerprint)
        unique_findings.append(f)

        f_path = findings_dir / f"{f.fuzzer}_{f.kind}_{f.fingerprint}.txt"
        f_content = [
            f"// FUZZER: {f.fuzzer}",
            f"// OS: {os_name}",
            f"// KIND: {f.kind}",
            f"// FINGERPRINT: {f.fingerprint}",
            f"// DESCRIPTION: {f.description}",
            "",
            f.program,
            "",
        ]
        if f.call_stack:
            f_content.extend([
                "/* CALL STACK:",
                f.call_stack,
                "*/",
                "",
            ])
        f_content.extend([
            "/* DETAILS:",
            f.details,
            "*/",
        ])
        f_path.write_text("\n".join(f_content) + "\n", encoding="utf-8")

    total_corpus = sum(corpus_counts.values())
    summary = {
        "mode": "libfuzzer",
        "os": os_name,
        "fuzzers": fuzzer_names,
        "timeout_seconds": args.timeout,
        "per_fuzzer_timeout_seconds": per_fuzzer_budget,
        "corpus_size": total_corpus,
        "corpus_by_fuzzer": corpus_counts,
        "findings_count": len(unique_findings),
        "findings": [
            {
                "fuzzer": f.fuzzer,
                "kind": f.kind,
                "fingerprint": f.fingerprint,
                "description": f.description,
                "program": f.program,
                "call_stack": f.call_stack,
                "details": f.details,
            }
            for f in unique_findings
        ],
    }

    summary_json = json.dumps(summary, indent=2) + "\n"
    (output_dir / "summary.json").write_text(summary_json, encoding="utf-8")
    (output_dir / f"summary-{os_name}.json").write_text(summary_json, encoding="utf-8")

    print("\n=== LibFuzzer Summary ===")
    print(f"OS                  : {os_name}")
    print(f"Fuzzers Run         : {', '.join(fuzzer_names)}")
    print(f"Total Corpus Inputs : {total_corpus} ({corpus_counts})")
    print(f"Findings Discovered : {len(unique_findings)}")
    for idx, f in enumerate(unique_findings, start=1):
        print(f"  [{idx}] ({f.kind}) [{f.fingerprint}] {f.description}")

    if args.file_github_issues and unique_findings:
        file_github_issues_for_findings(
            summary["findings"],
            repo=args.repo,
            run_url=args.run_url,
            dry_run=args.dry_run_issues,
        )

    return 0


if __name__ == "__main__":
    sys.exit(main())
