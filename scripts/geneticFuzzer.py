#!/usr/bin/env python3
# Copyright (c) 2022 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

"""Coverage-guided genetic fuzzer and differential tester for WarfLang.

Supports two operating modes:
  - corpus (default): Evolves a population and accumulates inputs that discover
    new line/region/branch coverage or trigger crashes, hangs, or differential
    divergences across Warf execution backends.
  - single: Evolves a single program to maximize the coverage percentage
    achieved by one input.

Supports grammar-aware and token-level mutations, seed loading from Warf
`.wf` test suites, fast coverage extraction via `llvm-cov export --summary-only`
with automatic cleanup of temporary `.profraw`/`.profdata` files, and optional
differential testing across `Warf --eval`, `Warf --show_tree`, `Warf --emit-ir`,
`WarfIR --run`, and `Warfc`.
"""

import argparse
from dataclasses import dataclass, field
import hashlib
import json
import os
from pathlib import Path
import random
import re
import shutil
import subprocess
import sys
import tempfile
import time
from typing import Dict, List, Optional, Sequence, Set, Tuple

DEFAULT_SEED = "a = (3*(2+1)/4 == 5*(10*(3+4)-10)) && true"

GENES = (
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "1234567890=+-*/&|!^()~<>:;{},_ \n"
)

IDENTIFIERS = ["a", "b", "c", "x", "y", "z", "value", "index", "res", "n", "_a"]
FUNC_NAMES = ["foo", "bar", "baz", "compute", "abs", "max", "clamp", "step"]
INT_LITERALS = ["0", "1", "2", "3", "4", "5", "7", "10", "42", "255", "2147483647", "2147483648"]
BOOL_LITERALS = ["true", "false"]
ARITH_OPS = ["+", "-", "*", "/", "&", "|", "^"]
CMP_OPS = ["==", "!=", "<", "<=", ">", ">="]
BOOL_OPS = ["&&", "||", "==", "!="]
UNARY_INT_OPS = ["-", "+", "~"]
UNARY_BOOL_OPS = ["!"]
ASSIGN_OPS = ["=", "+=", "-=", "*=", "/=", "&=", "|=", "^="]
TYPE_ANNOTATIONS = ["int", "bool", "number", "boolean"]

KEYWORDS_AND_TOKENS = [
    "let", "var", "if", "else", "while", "for", "function", "return",
    "true", "false", "int", "bool", "number", "boolean",
    "+", "-", "*", "/", "&", "|", "^", "~", "!", "=",
    "+=", "-=", "*=", "/=", "&=", "|=", "^=",
    "==", "!=", "<", "<=", ">", ">=", "&&", "||",
    "(", ")", "{", "}", ";", ":", ",", "//",
]

BUILTIN_TEMPLATES = [
    DEFAULT_SEED,
    "let a = 10; var b = 20; a + b * 2 - (b / 3)",
    "a = 4; a += 2; a -= 1; a *= 4; a /= 2; a &= 7; a |= 8; a ^= 5; ~a",
    "b1 = true; b2 = false; (b1 && !b2) || (1 < 2 && 3 >= 3) != false",
    "value = 0; if (1 < 2) { value = 4 } else { value = 5 }; value",
    "value = 0; while (value < 3) { value += 1 }; value",
    "value = 0; for (index = 0; index < 3; index += 1) { value += 2 }; value",
    "scope_outer = 1; { scope_Inner = 2; scope_outer = scope_Inner + 3 }; scope_outer",
    "function foo(a: int, b: int): int { if (a > b) { return a } else { return b } } foo(10, 20)",
    "function fact(n: int): int { if (n <= 1) { return 1 } return n * fact(n - 1) } fact(4)",
    "function isPos(x: int): bool { return x > 0 } res = 0; if (isPos(3) && !isPos(-1)) { res = 1 } res",
    "1 + 3 // trailing comment",
    # Edge cases and diagnostic paths (lexer, parser, binder, evaluator errors)
    "2147483648",
    "1 / 0",
    "!1",
    "1 == true",
    "missing_var += 1",
    "missing_fn(1, 2)",
    "function inc(x: int): int { return x + 1 } inc(true)",
    "function pair(a: int, b: int): int { return a + b } pair(1)",
    "a = ((1 + 2)",
    "1 ? 2",
]

TOKEN_REGEX = re.compile(
    r"//[^\n]*|"
    r"\+=|-=|\*=|/=|&=|\|=|\^=|==|!=|<=|>=|&&|\|\||"
    r"[A-Za-z_][A-Za-z0-9_]*|"
    r"\d+|"
    r"[+\-*/&|^~!()<>{};:,=]|"
    r"\S"
)


@dataclass
class CoverageMetrics:
    lines_covered: int = 0
    lines_total: int = 0
    lines_percent: float = 0.0
    regions_covered: int = 0
    regions_total: int = 0
    regions_percent: float = 0.0
    branches_covered: int = 0
    branches_total: int = 0
    branches_percent: float = 0.0
    functions_covered: int = 0
    functions_total: int = 0
    functions_percent: float = 0.0

    @property
    def combined_score(self) -> float:
        """Weighted coverage percentage prioritizing regions, lines, and branches."""
        if self.regions_total > 0 and self.branches_total > 0:
            return (
                0.45 * self.lines_percent
                + 0.40 * self.regions_percent
                + 0.15 * self.branches_percent
            )
        if self.regions_total > 0:
            return 0.55 * self.lines_percent + 0.45 * self.regions_percent
        return self.lines_percent


@dataclass
class Finding:
    kind: str  # "crash", "hang", "differential", "ir_error"
    program: str
    description: str
    details: str = ""


@dataclass
class EvaluationResult:
    program: str
    individual_coverage: CoverageMetrics = field(default_factory=CoverageMetrics)
    discovered_new_coverage: bool = False
    new_lines_delta: int = 0
    new_regions_delta: int = 0
    new_branches_delta: int = 0
    findings: List[Finding] = field(default_factory=list)
    exec_modes_run: int = 0
    fitness: float = 100.0


def tokenize_program(program: str) -> List[str]:
    return TOKEN_REGEX.findall(program)


def detokenize_program(tokens: Sequence[str]) -> str:
    if not tokens:
        return ""
    out: List[str] = []
    no_space_before = {";", ",", ")", ":", "}"}
    no_space_after = {"(", "~", "!"}
    for i, tok in enumerate(tokens):
        if i > 0:
            prev = tokens[i - 1]
            if prev.startswith("//"):
                out.append("\n")
            elif tok in no_space_before or prev in no_space_after:
                pass
            else:
                out.append(" ")
        out.append(tok)
    return "".join(out)


class WarfGenerator:
    """Grammar-aware generator and mutator for WarfLang programs."""

    def __init__(self, rng: Optional[random.Random] = None, max_len: int = 512):
        self.rng = rng or random.Random()
        self.max_len = max_len

    def random_int_expr(self, depth: int = 0, env_vars: Optional[List[str]] = None) -> str:
        vars_pool = env_vars if env_vars else ["a", "b", "x", "value"]
        if depth >= 3 or self.rng.random() < 0.35:
            choice = self.rng.random()
            if choice < 0.55:
                return self.rng.choice(INT_LITERALS[:-2])
            if choice < 0.85 and vars_pool:
                return self.rng.choice(vars_pool)
            return f"{self.rng.choice(UNARY_INT_OPS)}{self.rng.choice(INT_LITERALS[:-2])}"

        r = self.rng.random()
        if r < 0.20:
            op = self.rng.choice(UNARY_INT_OPS)
            return f"{op}({self.random_int_expr(depth + 1, vars_pool)})"
        if r < 0.80:
            op = self.rng.choice(ARITH_OPS)
            left = self.random_int_expr(depth + 1, vars_pool)
            right = self.random_int_expr(depth + 1, vars_pool)
            if op == "/" and right in ("0", "(0)"):
                right = "1"
            return f"({left} {op} {right})"
        var = self.rng.choice(vars_pool)
        assign_op = self.rng.choice(ASSIGN_OPS)
        return f"({var} {assign_op} {self.random_int_expr(depth + 1, vars_pool)})"

    def random_bool_expr(self, depth: int = 0, env_vars: Optional[List[str]] = None) -> str:
        vars_pool = env_vars if env_vars else ["a", "b", "x", "value"]
        if depth >= 3 or self.rng.random() < 0.30:
            return self.rng.choice(BOOL_LITERALS)

        r = self.rng.random()
        if r < 0.20:
            return f"!({self.random_bool_expr(depth + 1, vars_pool)})"
        if r < 0.65:
            op = self.rng.choice(CMP_OPS)
            left = self.random_int_expr(depth + 1, vars_pool)
            right = self.random_int_expr(depth + 1, vars_pool)
            return f"({left} {op} {right})"
        op = self.rng.choice(BOOL_OPS)
        left = self.random_bool_expr(depth + 1, vars_pool)
        right = self.random_bool_expr(depth + 1, vars_pool)
        return f"({left} {op} {right})"

    def random_statement(self, depth: int = 0, env_vars: Optional[List[str]] = None, in_func: bool = False) -> str:
        if env_vars is None:
            env_vars = ["a", "b", "value"]

        if depth >= 2:
            var = self.rng.choice(env_vars)
            op = self.rng.choice(ASSIGN_OPS)
            return f"{var} {op} {self.random_int_expr(depth + 1, env_vars)}"

        r = self.rng.random()
        if r < 0.25:
            kw = self.rng.choice(["let", "var", ""])
            var = self.rng.choice(IDENTIFIERS)
            if var not in env_vars and kw != "let":
                env_vars.append(var)
            expr = (
                self.random_int_expr(depth + 1, env_vars)
                if self.rng.random() < 0.8
                else self.random_bool_expr(depth + 1, env_vars)
            )
            return f"{kw} {var} = {expr}".strip()
        if r < 0.45:
            var = self.rng.choice(env_vars)
            op = self.rng.choice(ASSIGN_OPS)
            return f"{var} {op} {self.random_int_expr(depth + 1, env_vars)}"
        if r < 0.62:
            cond = self.random_bool_expr(depth + 1, env_vars)
            then_stmt = self.random_statement(depth + 1, env_vars, in_func=in_func)
            if self.rng.random() < 0.65:
                else_stmt = self.random_statement(depth + 1, env_vars, in_func=in_func)
                return f"if ({cond}) {{ {then_stmt} }} else {{ {else_stmt} }}"
            return f"if ({cond}) {{ {then_stmt} }}"
        if r < 0.74:
            loop_var = self.rng.choice(["i", "index", "c"])
            limit = self.rng.randint(1, 5)
            body = self.random_statement(depth + 1, env_vars, in_func=in_func)
            return f"{loop_var} = 0; while ({loop_var} < {limit}) {{ {body}; {loop_var} += 1 }}"
        if r < 0.86:
            idx = self.rng.choice(["index", "z"])
            limit = self.rng.randint(1, 5)
            body = self.random_statement(depth + 1, env_vars, in_func=in_func)
            return f"for ({idx} = 0; {idx} < {limit}; {idx} += 1) {{ {body} }}"
        if r < 0.94 and not in_func:
            fn = self.rng.choice(FUNC_NAMES)
            p1, p2 = "x", "y"
            local_vars = [p1, p2]
            use_types = self.rng.random() < 0.7
            sig = f"{p1}: int, {p2}: int" if use_types else f"{p1}, {p2}"
            ret_annot = ": int" if use_types else ""
            body_stmt = self.random_statement(depth + 1, local_vars, in_func=True)
            ret_expr = self.random_int_expr(depth + 1, local_vars)
            arg1 = self.random_int_expr(depth + 1, env_vars)
            arg2 = self.random_int_expr(depth + 1, env_vars)
            return f"function {fn}({sig}){ret_annot} {{ {body_stmt}; return {ret_expr} }} {fn}({arg1}, {arg2})"
        if in_func and self.rng.random() < 0.5:
            return f"return {self.random_int_expr(depth + 1, env_vars)}"
        return f"{{ {self.random_statement(depth + 1, env_vars, in_func=in_func)} }}"

    def generate_program(self) -> str:
        if self.rng.random() < 0.25:
            return self.rng.choice(BUILTIN_TEMPLATES)

        env_vars = ["a", "b", "value"]
        stmts = [
            "a = " + self.rng.choice(["0", "1", "2", "5"]),
            "b = " + self.rng.choice(["1", "3", "4", "10"]),
            "value = 0",
        ]
        num_stmts = self.rng.randint(1, 4)
        for _ in range(num_stmts):
            stmts.append(self.random_statement(0, env_vars))
        stmts.append(self.random_int_expr(1, env_vars))
        prog = "; ".join(stmts)
        return prog[: self.max_len]

    def mutate(self, program: str) -> str:
        """Apply one or more grammar-aware, token-level, or character-level mutations."""
        if not program:
            return self.generate_program()

        strategy = self.rng.random()
        tokens = tokenize_program(program)

        # 1. Token-level mutation (50%)
        if tokens and strategy < 0.50:
            op = self.rng.choice(["replace", "insert", "delete", "swap", "append_stmt"])
            if op == "replace":
                idx = self.rng.randrange(len(tokens))
                tok = tokens[idx]
                if tok in ARITH_OPS:
                    tokens[idx] = self.rng.choice(ARITH_OPS)
                elif tok in CMP_OPS:
                    tokens[idx] = self.rng.choice(CMP_OPS)
                elif tok in BOOL_OPS:
                    tokens[idx] = self.rng.choice(BOOL_OPS)
                elif tok in ASSIGN_OPS:
                    tokens[idx] = self.rng.choice(ASSIGN_OPS)
                elif tok in BOOL_LITERALS:
                    tokens[idx] = "false" if tok == "true" else "true"
                elif tok.isdigit():
                    tokens[idx] = self.rng.choice(INT_LITERALS)
                elif tok in IDENTIFIERS:
                    tokens[idx] = self.rng.choice(IDENTIFIERS)
                elif tok in TYPE_ANNOTATIONS:
                    tokens[idx] = self.rng.choice(TYPE_ANNOTATIONS)
                else:
                    tokens[idx] = self.rng.choice(KEYWORDS_AND_TOKENS)
            elif op == "insert":
                idx = self.rng.randrange(len(tokens) + 1)
                tokens.insert(idx, self.rng.choice(KEYWORDS_AND_TOKENS + IDENTIFIERS + INT_LITERALS))
            elif op == "delete" and len(tokens) > 1:
                idx = self.rng.randrange(len(tokens))
                del tokens[idx]
            elif op == "swap" and len(tokens) > 2:
                i, j = self.rng.sample(range(len(tokens)), 2)
                tokens[i], tokens[j] = tokens[j], tokens[i]
            elif op == "append_stmt":
                extra = tokenize_program("; " + self.random_statement(1))
                tokens.extend(extra)
            mutated = detokenize_program(tokens)
            return mutated[: self.max_len] if mutated else self.generate_program()

        # 2. Grammar-level splice/wrap mutation (30%)
        if strategy < 0.80:
            sub_choice = self.rng.random()
            if sub_choice < 0.35:
                return f"{program}; {self.random_statement(1)}"[: self.max_len]
            if sub_choice < 0.65:
                cond = self.random_bool_expr(1)
                return f"if ({cond}) {{ {program} }} else {{ {self.random_statement(1)} }}"[: self.max_len]
            if sub_choice < 0.85:
                return f"{{ {program} }}"[: self.max_len]
            # Inject an edge-case snippet to trigger error/diagnostic paths
            edge = self.rng.choice(BUILTIN_TEMPLATES[-9:])
            return f"{program}; {edge}"[: self.max_len]

        # 3. Character-level mutation for lexer/parser fuzzing (20%)
        chars = list(program)
        num_edits = self.rng.randint(1, max(1, min(4, len(chars))))
        for _ in range(num_edits):
            edit = self.rng.choice(["replace", "insert", "delete"])
            if edit == "replace" and chars:
                chars[self.rng.randrange(len(chars))] = self.rng.choice(GENES)
            elif edit == "insert":
                chars.insert(self.rng.randrange(len(chars) + 1), self.rng.choice(GENES))
            elif edit == "delete" and len(chars) > 1:
                del chars[self.rng.randrange(len(chars))]
        return "".join(chars)[: self.max_len]

    def crossover(self, parent1: str, parent2: str) -> str:
        """Combine two parent programs at statement or token boundaries."""
        if not parent1:
            return parent2
        if not parent2:
            return parent1

        # Statement-level crossover if semicolons or newlines are present
        stmts1 = [s.strip() for s in re.split(r"[;\n]+", parent1) if s.strip()]
        stmts2 = [s.strip() for s in re.split(r"[;\n]+", parent2) if s.strip()]
        if len(stmts1) > 1 and len(stmts2) > 1 and self.rng.random() < 0.6:
            cut1 = self.rng.randint(1, len(stmts1) - 1)
            cut2 = self.rng.randint(1, len(stmts2) - 1)
            combined = stmts1[:cut1] + stmts2[cut2:]
            child = "; ".join(combined)
        else:
            t1 = tokenize_program(parent1)
            t2 = tokenize_program(parent2)
            if t1 and t2:
                c1 = self.rng.randint(0, len(t1))
                c2 = self.rng.randint(0, len(t2))
                child = detokenize_program(t1[:c1] + t2[c2:])
            else:
                mid = len(parent1) // 2
                child = parent1[:mid] + parent2[mid:]

        if self.rng.random() < 0.35:
            child = self.mutate(child)
        return (child or parent1)[: self.max_len]


def load_seeds_from_directory(seed_dir: Path) -> List[str]:
    """Load `.wf` programs from a directory (e.g. `test/filecheck`), stripping lit comments."""
    seeds: List[str] = []
    if not seed_dir.exists():
        return seeds

    for wf_path in sorted(seed_dir.rglob("*.wf")):
        try:
            raw_lines = wf_path.read_text(encoding="utf-8", errors="ignore").splitlines()
        except OSError:
            continue
        code_lines: List[str] = []
        for line in raw_lines:
            stripped = line.strip()
            if (
                stripped.startswith("// RUN:")
                or stripped.startswith("// REQUIRES:")
                or stripped.startswith("// CHECK")
            ):
                continue
            code_lines.append(line)
        program = "\n".join(code_lines).strip()
        if program:
            seeds.append(program)
    return seeds


def extract_covered_segments(export_json: dict) -> Tuple[CoverageMetrics, Set[Tuple[str, int, int]]]:
    """Parse `llvm-cov export` JSON output into summary metrics and covered segments."""
    metrics = CoverageMetrics()
    segments_hit: Set[Tuple[str, int, int]] = set()

    data_list = export_json.get("data", [])
    if not data_list:
        return metrics, segments_hit

    first_data = data_list[0]
    totals = first_data.get("totals", {})

    lines_info = totals.get("lines", {})
    metrics.lines_covered = int(lines_info.get("covered", 0))
    metrics.lines_total = int(lines_info.get("count", 0))
    metrics.lines_percent = float(lines_info.get("percent", 0.0))

    regions_info = totals.get("regions", {})
    metrics.regions_covered = int(regions_info.get("covered", 0))
    metrics.regions_total = int(regions_info.get("count", 0))
    metrics.regions_percent = float(regions_info.get("percent", 0.0))

    branches_info = totals.get("branches", {})
    metrics.branches_covered = int(branches_info.get("covered", 0))
    metrics.branches_total = int(branches_info.get("count", 0))
    metrics.branches_percent = float(branches_info.get("percent", 0.0))

    functions_info = totals.get("functions", {})
    metrics.functions_covered = int(functions_info.get("covered", 0))
    metrics.functions_total = int(functions_info.get("count", 0))
    metrics.functions_percent = float(functions_info.get("percent", 0.0))

    for file_entry in first_data.get("files", []):
        filename = file_entry.get("filename", "")
        for seg in file_entry.get("segments", []):
            # Segment format: [line, col, count, hasCount, isRegionEntry, ...]
            if len(seg) >= 5 and seg[3] and seg[2] > 0:
                segments_hit.add((filename, int(seg[0]), int(seg[1])))

    return metrics, segments_hit


def parse_llvm_cov_report_fallback(report_stdout: str) -> CoverageMetrics:
    """Fallback parser for `llvm-cov report` text output."""
    metrics = CoverageMetrics()
    lines = [ln.strip() for ln in report_stdout.splitlines() if ln.strip()]
    if not lines:
        return metrics
    last_line = lines[-1]
    pct_matches = re.findall(r"(\d+(?:\.\d+)?)%", last_line)
    if len(pct_matches) >= 3:
        metrics.regions_percent = float(pct_matches[0])
        metrics.functions_percent = float(pct_matches[1])
        metrics.lines_percent = float(pct_matches[2])
        if len(pct_matches) >= 4:
            metrics.branches_percent = float(pct_matches[3])
    elif pct_matches:
        metrics.lines_percent = float(pct_matches[-1])
    return metrics


def is_sanitizer_or_crash(returncode: int, stderr: str) -> bool:
    if returncode < 0:
        return True
    # Signals on Unix shell often map to 128 + sig (e.g., 134 = SIGABRT, 139 = SIGSEGV)
    if returncode in (134, 136, 139):
        return True
    crash_markers = (
        "AddressSanitizer",
        "UndefinedBehaviorSanitizer",
        "MemorySanitizer",
        "Assertion failed",
        "assertion failed",
        "Segmentation fault",
        "Abort trap",
        "LLVM backend error:",
        "IR parse error:",
        "IR error:",
        "EvaluatorError:",
        "Unexpected syntax",
        "Unexpected SyntaxKind",
    )
    return any(marker in stderr for marker in crash_markers)


class WarfFuzzerHarness:
    """Executes Warf binaries, collects LLVM coverage, and performs differential testing."""

    def __init__(
        self,
        llvm_path: str,
        warf_cli_path: str,
        warf_ir_path: Optional[str] = None,
        warfc_path: Optional[str] = None,
        mode: str = "corpus",
        differential: bool = True,
        cmd_timeout: float = 2.0,
        work_dir: Optional[Path] = None,
    ):
        self.llvm_dir = Path(llvm_path) if llvm_path else Path()
        self.llvm_profdata = self._resolve_tool("llvm-profdata")
        self.llvm_cov = self._resolve_tool("llvm-cov")

        self.warf_cli = Path(warf_cli_path).resolve()
        cli_dir = self.warf_cli.parent

        self.warf_ir = self._resolve_sibling(warf_ir_path, [
            cli_dir / "WarfIR",
            cli_dir.parent / "irTool" / "WarfIR",
        ])
        self.warfc = self._resolve_sibling(warfc_path, [
            cli_dir / "Warfc",
            cli_dir.parent / "compilerTool" / "Warfc",
        ])

        self.mode = mode
        self.differential = differential
        self.cmd_timeout = cmd_timeout
        self._owns_work_dir = work_dir is None
        self.work_dir = work_dir or Path(tempfile.mkdtemp(prefix="warf_fuzz_"))
        self.work_dir.mkdir(parents=True, exist_ok=True)
        self.cumulative_profdata = self.work_dir / "cumulative.profdata"

        self.runs = 0
        self.cumulative_metrics = CoverageMetrics()
        self.seen_segments: Set[Tuple[str, int, int]] = set()
        self.seen_Error_signatures: Set[str] = set()
        self.corpus: List[Tuple[str, CoverageMetrics]] = []
        self.findings: List[Finding] = []
        self._finding_hashes: Set[str] = set()
        self.coverage_supported = True

    def _resolve_tool(self, tool_name: str) -> str:
        if self.llvm_dir and (self.llvm_dir / tool_name).exists():
            return str((self.llvm_dir / tool_name).resolve())
        found = shutil.which(tool_name)
        if found:
            return found
        for brew_prefix in ("/opt/homebrew/opt/llvm/bin", "/usr/local/opt/llvm/bin"):
            candidate = Path(brew_prefix) / tool_name
            if candidate.exists():
                return str(candidate)
        return str(self.llvm_dir / tool_name)

    @staticmethod
    def _resolve_sibling(explicit: Optional[str], candidates: Sequence[Path]) -> Optional[Path]:
        if explicit:
            p = Path(explicit).resolve()
            return p if p.exists() else None
        for cand in candidates:
            if cand.exists() and os.access(cand, os.X_OK):
                return cand.resolve()
        return None

    def close(self) -> None:
        if self._owns_work_dir and self.work_dir.exists():
            shutil.rmtree(self.work_dir, ignore_errors=True)

    def _record_finding(self, kind: str, program: str, description: str, details: str = "") -> Optional[Finding]:
        sig = hashlib.sha256(f"{kind}:{description}".encode("utf-8")).hexdigest()[:16]
        if sig in self._finding_hashes:
            return None
        self._finding_hashes.add(sig)
        finding = Finding(kind=kind, program=program, description=description, details=details)
        self.findings.append(finding)
        return finding

    def _run_cmd(
        self, cmd: Sequence[str], env: Dict[str, str], timeout: Optional[float] = None
    ) -> Tuple[Optional[int], str, str, bool]:
        try:
            proc = subprocess.run(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=env,
                timeout=timeout or self.cmd_timeout,
                text=True,
                errors="replace",
            )
            return proc.returncode, proc.stdout, proc.stderr, False
        except subprocess.TimeoutExpired as exc:
            out = exc.stdout.decode("utf-8", errors="replace") if isinstance(exc.stdout, bytes) else (exc.stdout or "")
            err = exc.stderr.decode("utf-8", errors="replace") if isinstance(exc.stderr, bytes) else (exc.stderr or "")
            return None, out, err, True
        except OSError as exc:
            return -1, "", str(exc), False

    def _query_coverage(
        self, profdata_path: Path, objects: Sequence[Path], summary_only: bool = True
    ) -> Tuple[CoverageMetrics, Set[Tuple[str, int, int]]]:
        cmd = [
            self.llvm_cov,
            "export",
            f"-instr-profile={profdata_path}",
            str(objects[0]),
        ]
        for extra_obj in objects[1:]:
            cmd.extend(["-object", str(extra_obj)])
        if summary_only:
            cmd.append("--summary-only")

        try:
            proc = subprocess.run(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                timeout=5.0,
                text=True,
                errors="replace",
            )
            if proc.returncode == 0 and proc.stdout.strip():
                data = json.loads(proc.stdout)
                return extract_covered_segments(data)
        except (subprocess.SubprocessError, json.JSONDecodeError, OSError):
            pass

        # Fallback to `llvm-cov report`
        rep_cmd = [
            self.llvm_cov,
            "report",
            f"-instr-profile={profdata_path}",
            f"-object={objects[0]}",
        ]
        try:
            proc = subprocess.run(
                rep_cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                timeout=5.0,
                text=True,
                errors="replace",
            )
            if proc.returncode == 0:
                return parse_llvm_cov_report_fallback(proc.stdout), set()
        except (subprocess.SubprocessError, OSError):
            pass

        return CoverageMetrics(), set()

    def evaluate_program(self, program: str) -> EvaluationResult:
        self.runs += 1
        run_id = self.runs
        result = EvaluationResult(program=program)

        profraw_pattern = str(self.work_dir / f"run_{run_id}_%p_%m.profraw")
        env = os.environ.copy()
        env["LLVM_PROFILE_FILE"] = profraw_pattern

        used_objects: List[Path] = [self.warf_cli]
        cov_objects: List[Path] = [self.warf_cli]
        if self.warf_ir:
            cov_objects.append(self.warf_ir)
        if self.warfc:
            cov_objects.append(self.warfc)

        # 1. Primary execution: Warf --eval <program>
        rc_eval, out_eval, err_eval, timed_out = self._run_cmd(
            [str(self.warf_cli), "--eval", program], env
        )
        result.exec_modes_run += 1

        if timed_out:
            f = self._record_finding("hang", program, "Timeout in `Warf --eval`")
            if f:
                result.findings.append(f)
        elif rc_eval is not None and is_sanitizer_or_crash(rc_eval, err_eval):
            f = self._record_finding(
                "crash",
                program,
                f"Crash/Unhandled error in `Warf --eval` (exit {rc_eval})",
                err_eval.strip(),
            )
            if f:
                result.findings.append(f)

        # Also exercise `--show_tree` periodically or when clean to cover AST printing
        rc_tree, _, err_tree, tree_timeout = self._run_cmd(
            [str(self.warf_cli), "--eval", program, "--show_tree"], env
        )
        result.exec_modes_run += 1
        if not tree_timeout and rc_tree is not None and is_sanitizer_or_crash(rc_tree, err_tree):
            f = self._record_finding(
                "crash",
                program,
                f"Crash in `Warf --eval --show_tree` (exit {rc_tree})",
                err_tree.strip(),
            )
            if f:
                result.findings.append(f)

        # 2. Differential & IR pipeline testing if `--eval` succeeded without diagnostics
        eval_clean = (
            not timed_out
            and rc_eval == 0
            and not err_eval.strip()
        )
        if self.differential or eval_clean:
            rc_ir, out_ir, err_ir, ir_timeout = self._run_cmd(
                [str(self.warf_cli), "--eval", program, "--emit-ir"], env
            )
            result.exec_modes_run += 1
            if ir_timeout:
                f = self._record_finding("hang", program, "Timeout in `Warf --emit-ir`")
                if f:
                    result.findings.append(f)
            elif rc_ir is not None and is_sanitizer_or_crash(rc_ir, err_ir):
                f = self._record_finding(
                    "crash",
                    program,
                    f"Crash/Error in `Warf --emit-ir` (exit {rc_ir})",
                    err_ir.strip(),
                )
                if f:
                    result.findings.append(f)
            elif eval_clean and rc_ir == 0 and out_ir.strip() and self.warf_ir:
                # Run the emitted WarfIR through `WarfIR --run` and compare with `Warf --eval`
                ir_file = self.work_dir / f"input_{run_id}.wir"
                try:
                    ir_file.write_text(out_ir, encoding="utf-8")
                    used_objects.append(self.warf_ir)
                    rc_wir, out_wir, err_wir, wir_timeout = self._run_cmd(
                        [str(self.warf_ir), "--run", str(ir_file)], env
                    )
                    result.exec_modes_run += 1
                    if wir_timeout:
                        f = self._record_finding("hang", program, "Timeout in `WarfIR --run`", out_ir)
                        if f:
                            result.findings.append(f)
                    elif rc_wir is not None and is_sanitizer_or_crash(rc_wir, err_wir):
                        f = self._record_finding(
                            "ir_error",
                            program,
                            f"`WarfIR --run` failed on emitted IR (exit {rc_wir})",
                            f"Stderr:\n{err_wir.strip()}\n\nEmitted IR:\n{out_ir}",
                        )
                        if f:
                            result.findings.append(f)
                    elif rc_wir == 0 and not err_wir.strip():
                        eval_val = out_eval.strip()
                        wir_val = out_wir.strip()
                        if eval_val != wir_val:
                            f = self._record_finding(
                                "differential",
                                program,
                                f"Mismatch between `Warf --eval` ({eval_val!r}) and `WarfIR --run` ({wir_val!r})",
                                f"Emitted IR:\n{out_ir}",
                            )
                            if f:
                                result.findings.append(f)

                    # Also exercise `WarfIR --emit-llvm-ir` if available
                    rc_ll, _, err_ll, ll_timeout = self._run_cmd(
                        [str(self.warf_ir), "--emit-llvm-ir", str(ir_file)], env
                    )
                    result.exec_modes_run += 1
                    if (
                        not ll_timeout
                        and rc_ll is not None
                        and "LLVM backend is not available" not in err_ll
                        and is_sanitizer_or_crash(rc_ll, err_ll)
                    ):
                        f = self._record_finding(
                            "ir_error",
                            program,
                            f"`WarfIR --emit-llvm-ir` error (exit {rc_ll})",
                            f"Stderr:\n{err_ll.strip()}\n\nEmitted IR:\n{out_ir}",
                        )
                        if f:
                            result.findings.append(f)
                finally:
                    ir_file.unlink(missing_ok=True)

            # Exercise `Warfc` on clean programs if Warfc binary exists
            if eval_clean and self.warfc:
                wf_file = self.work_dir / f"prog_{run_id}.wf"
                exe_file = self.work_dir / f"prog_{run_id}.exe"
                try:
                    wf_file.write_text(program + "\n", encoding="utf-8")
                    used_objects.append(self.warfc)
                    rc_wc, _, err_wc, wc_timeout = self._run_cmd(
                        [str(self.warfc), "-o", str(exe_file), str(wf_file)], env
                    )
                    result.exec_modes_run += 1
                    if wc_timeout:
                        f = self._record_finding("hang", program, "Timeout in `Warfc`")
                        if f:
                            result.findings.append(f)
                    elif (
                        rc_wc is not None
                        and "LLVM backend is not available" not in err_wc
                        and is_sanitizer_or_crash(rc_wc, err_wc)
                    ):
                        f = self._record_finding(
                            "crash",
                            program,
                            f"`Warfc` failed (exit {rc_wc})",
                            err_wc.strip(),
                        )
                        if f:
                            result.findings.append(f)
                finally:
                    wf_file.unlink(missing_ok=True)
                    exe_file.unlink(missing_ok=True)

        # 3. Merge `.profraw` files and compute coverage
        profraw_files = list(self.work_dir.glob(f"run_{run_id}_*.profraw"))
        run_profdata = self.work_dir / f"run_{run_id}.profdata"
        try:
            if profraw_files and self.coverage_supported:
                merge_cmd = [
                    self.llvm_profdata,
                    "merge",
                    "-sparse",
                    *[str(p) for p in profraw_files],
                    "-o",
                    str(run_profdata),
                ]
                proc = subprocess.run(
                    merge_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=5.0
                )
                if proc.returncode == 0 and run_profdata.exists():
                    indiv_metrics, segments = self._query_coverage(
                        run_profdata, used_objects, summary_only=(self.mode == "single")
                    )
                    result.individual_coverage = indiv_metrics

                    new_segs = segments - self.seen_segments
                    if new_segs:
                        self.seen_segments.update(new_segs)
                        result.discovered_new_coverage = True

                    # Update cumulative profdata
                    merge_inputs = [str(run_profdata)]
                    if self.cumulative_profdata.exists():
                        merge_inputs.append(str(self.cumulative_profdata))
                    cum_tmp = self.work_dir / "cumulative_next.profdata"
                    cum_cmd = [
                        self.llvm_profdata,
                        "merge",
                        "-sparse",
                        *merge_inputs,
                        "-o",
                        str(cum_tmp),
                    ]
                    cproc = subprocess.run(
                        cum_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=5.0
                    )
                    if cproc.returncode == 0 and cum_tmp.exists():
                        cum_tmp.replace(self.cumulative_profdata)
                        prev = self.cumulative_metrics
                        cum_metrics, _ = self._query_coverage(
                            self.cumulative_profdata, cov_objects, summary_only=True
                        )
                        result.new_lines_delta = max(0, cum_metrics.lines_covered - prev.lines_covered)
                        result.new_regions_delta = max(0, cum_metrics.regions_covered - prev.regions_covered)
                        result.new_branches_delta = max(0, cum_metrics.branches_covered - prev.branches_covered)
                        if (
                            result.new_lines_delta > 0
                            or result.new_regions_delta > 0
                            or result.new_branches_delta > 0
                        ):
                            result.discovered_new_coverage = True
                        self.cumulative_metrics = cum_metrics
                else:
                    self.coverage_supported = False
            else:
                self.coverage_supported = False
        except (subprocess.SubprocessError, OSError):
            self.coverage_supported = False
        finally:
            for p in profraw_files:
                p.unlink(missing_ok=True)
            run_profdata.unlink(missing_ok=True)

        # Fallback behavioral novelty when running against an uninstrumented binary
        if not self.coverage_supported:
            diag_sig = ""
            for stream_txt in (err_eval, out_eval):
                for line in stream_txt.splitlines():
                    norm = re.sub(r"\d+", "N", line.strip())
                    if norm:
                        diag_sig = norm[:80]
                        break
                if diag_sig:
                    break
            sig_key = f"rc={rc_eval}|clean={eval_clean}|diag={diag_sig}"
            if sig_key not in self.seen_Error_signatures:
                self.seen_Error_signatures.add(sig_key)
                result.discovered_new_coverage = True

        # 4. Compute fitness (lower is better, 0.0 is ideal)
        cov_score = result.individual_coverage.combined_score
        if self.mode == "single":
            result.fitness = max(0.0, 100.0 - cov_score)
        else:
            # In corpus mode, reward individual coverage + bonus for new cumulative coverage or findings
            bonus = 0.0
            if result.discovered_new_coverage:
                bonus += 15.0 + 2.0 * (result.new_lines_delta + result.new_regions_delta)
            if result.findings:
                bonus += 25.0 * len(result.findings)
            if not self.coverage_supported and eval_clean:
                cov_score = min(60.0, len(tokenize_program(program)) * 1.5)
            result.fitness = max(0.0, 100.0 - (cov_score + bonus))

        if result.discovered_new_coverage or not self.corpus:
            self.corpus.append((program, result.individual_coverage))

        return result


# Backwards-compatible helper for callers importing `run_llvm_cov` directly
LLVM_PATH = ""
WARF_CLI_PATH = ""
TargetLen = len(DEFAULT_SEED)


def run_llvm_cov(chromosome: str) -> float:
    """Run `Warf -e <chromosome>` once and return line coverage percentage."""
    harness = WarfFuzzerHarness(
        llvm_path=LLVM_PATH,
        warf_cli_path=WARF_CLI_PATH,
        mode="single",
        differential=False,
    )
    try:
        res = harness.evaluate_program(chromosome)
        return res.individual_coverage.lines_percent
    finally:
        harness.close()


class Individual:
    """Individual program in the genetic population."""

    def __init__(
        self,
        chromosome: Sequence[str] | str,
        harness: Optional[WarfFuzzerHarness] = None,
        generator: Optional[WarfGenerator] = None,
    ):
        if isinstance(chromosome, list):
            self.program = "".join(chromosome)
        else:
            self.program = str(chromosome)
        self.chromosome = list(self.program)
        self.harness = harness
        self.generator = generator or WarfGenerator(max_len=max(TargetLen, 256))
        self.eval_result: Optional[EvaluationResult] = None
        self.fitness = self.cal_fitness()

    @classmethod
    def mutated_genes(cls) -> str:
        return random.choice(GENES)

    @classmethod
    def create_gnome(cls, generator: Optional[WarfGenerator] = None) -> str:
        gen = generator or WarfGenerator(max_len=max(TargetLen, 256))
        return gen.generate_program()

    def mate(self, par2: "Individual") -> "Individual":
        child_prog = self.generator.crossover(self.program, par2.program)
        if random.random() < 0.45:
            child_prog = self.generator.mutate(child_prog)
        return Individual(child_prog, harness=self.harness, generator=self.generator)

    def cal_fitness(self) -> float:
        if self.harness is not None:
            self.eval_result = self.harness.evaluate_program(self.program)
            return self.eval_result.fitness
        return 100.0 - run_llvm_cov(self.program)


def save_outputs(
    output_dir: Path,
    harness: WarfFuzzerHarness,
    best_individual: Optional[Individual],
) -> None:
    """Persist interesting corpus inputs, crash/differential reproducers, and summary JSON."""
    corpus_dir = output_dir / "corpus"
    findings_dir = output_dir / "findings"
    corpus_dir.mkdir(parents=True, exist_ok=True)
    findings_dir.mkdir(parents=True, exist_ok=True)

    for idx, (prog, cov) in enumerate(harness.corpus, start=1):
        digest = hashlib.sha256(prog.encode("utf-8")).hexdigest()[:10]
        wf_file = corpus_dir / f"id_{idx:04d}_{digest}.wf"
        header = (
            f"// Lines: {cov.lines_percent:.2f}% ({cov.lines_covered}/{cov.lines_total}) "
            f"Regions: {cov.regions_percent:.2f}%\n"
        )
        wf_file.write_text(header + prog + "\n", encoding="utf-8")

    for idx, finding in enumerate(harness.findings, start=1):
        digest = hashlib.sha256(finding.program.encode("utf-8")).hexdigest()[:10]
        f_path = findings_dir / f"{finding.kind}_{idx:03d}_{digest}.wf"
        content = [
            f"// KIND: {finding.kind}",
            f"// DESCRIPTION: {finding.description}",
        ]
        if finding.details:
            for line in finding.details.splitlines():
                content.append(f"// {line}")
        content.append("")
        content.append(finding.program)
        f_path.write_text("\n".join(content) + "\n", encoding="utf-8")

    summary = {
        "mode": harness.mode,
        "total_runs": harness.runs,
        "corpus_size": len(harness.corpus),
        "findings_count": len(harness.findings),
        "cumulative_coverage": {
            "lines_covered": harness.cumulative_metrics.lines_covered,
            "lines_total": harness.cumulative_metrics.lines_total,
            "lines_percent": harness.cumulative_metrics.lines_percent,
            "regions_covered": harness.cumulative_metrics.regions_covered,
            "regions_total": harness.cumulative_metrics.regions_total,
            "regions_percent": harness.cumulative_metrics.regions_percent,
            "branches_covered": harness.cumulative_metrics.branches_covered,
            "branches_total": harness.cumulative_metrics.branches_total,
            "branches_percent": harness.cumulative_metrics.branches_percent,
        },
        "best_single_program": best_individual.program if best_individual else "",
        "best_single_coverage_percent": (
            best_individual.eval_result.individual_coverage.lines_percent
            if best_individual and best_individual.eval_result
            else 0.0
        ),
        "findings": [
            {
                "kind": f.kind,
                "description": f.description,
                "program": f.program,
            }
            for f in harness.findings
        ],
    }
    (output_dir / "summary.json").write_text(
        json.dumps(summary, indent=2) + "\n", encoding="utf-8"
    )


def build_arg_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="geneticFuzzer",
        description="Coverage-guided genetic fuzzer and differential tester for WarfLang.",
    )
    p.add_argument(
        "llvm_path",
        type=str,
        help="Path to LLVM bin directory containing llvm-profdata and llvm-cov.",
    )
    p.add_argument(
        "warf_cli_path",
        type=str,
        help="Path to the instrumented Warf CLI executable.",
    )
    p.add_argument(
        "--warf-ir-path",
        dest="warf_ir_path",
        type=str,
        default=None,
        help="Optional path to WarfIR executable (auto-detected alongside Warf if omitted).",
    )
    p.add_argument(
        "--warfc-path",
        dest="warfc_path",
        type=str,
        default=None,
        help="Optional path to Warfc executable (auto-detected alongside Warf if omitted).",
    )
    p.add_argument(
        "-m",
        "--mode",
        choices=["corpus", "single"],
        default="corpus",
        help="Fuzzing mode: 'corpus' (default) maximizes cumulative corpus coverage and bug discovery; "
        "'single' optimizes coverage of a single input program.",
    )
    p.add_argument(
        "-p",
        "--population-size",
        dest="population_size",
        type=int,
        default=24,
        help="Number of individuals per generation (default: 24).",
    )
    p.add_argument(
        "-g",
        "--generations",
        type=int,
        default=20,
        help="Maximum number of generations to evolve (default: 20).",
    )
    p.add_argument(
        "--timeout",
        type=float,
        default=120.0,
        help="Overall fuzzing wall-clock timeout in seconds (default: 120).",
    )
    p.add_argument(
        "--cmd-timeout",
        dest="cmd_timeout",
        type=float,
        default=2.0,
        help="Per-command execution timeout in seconds (default: 2.0).",
    )
    p.add_argument(
        "--stagnation-limit",
        dest="stagnation_limit",
        type=int,
        default=8,
        help="Inject fresh grammar-generated immigrants after N generations without new coverage (default: 8).",
    )
    p.add_argument(
        "-t",
        "--target_length",
        dest="targetLen",
        type=int,
        default=512,
        help="Maximum length of generated/mutated Warf programs (default: 512).",
    )
    p.add_argument(
        "-s",
        "--seed",
        dest="startingSeed",
        type=str,
        default=DEFAULT_SEED,
        help="Initial seed expression to include in generation 1.",
    )
    p.add_argument(
        "--seed-dir",
        dest="seed_dir",
        type=str,
        default="test/filecheck",
        help="Directory of `.wf` test files to seed initial population (default: test/filecheck).",
    )
    p.add_argument(
        "--output-dir",
        dest="output_dir",
        type=str,
        default=None,
        help="Optional directory to save corpus `.wf` files, crash/divergence reproducers, and summary.json.",
    )
    p.add_argument(
        "--no-differential",
        dest="differential",
        action="store_false",
        default=True,
        help="Disable differential testing across Warf, WarfIR, and Warfc.",
    )
    p.add_argument(
        "--random-seed",
        dest="random_seed",
        type=int,
        default=None,
        help="Optional RNG seed for reproducible fuzzing runs.",
    )
    return p


def run_fuzzer(args: argparse.Namespace) -> int:
    global LLVM_PATH, WARF_CLI_PATH, TargetLen
    LLVM_PATH = args.llvm_path
    WARF_CLI_PATH = args.warf_cli_path
    TargetLen = args.targetLen

    rng = random.Random(args.random_seed)
    if args.random_seed is not None:
        random.seed(args.random_seed)

    generator = WarfGenerator(rng=rng, max_len=args.targetLen)
    harness = WarfFuzzerHarness(
        llvm_path=args.llvm_path,
        warf_cli_path=args.warf_cli_path,
        warf_ir_path=args.warf_ir_path,
        warfc_path=args.warfc_path,
        mode=args.mode,
        differential=args.differential,
        cmd_timeout=args.cmd_timeout,
    )

    start_time = time.monotonic()
    best_individual: Optional[Individual] = None

    try:
        # Assemble seed pool from CLI seed, `.wf` files in seed_dir, and built-in grammar templates
        seed_programs: List[str] = []
        if args.startingSeed:
            seed_programs.append(args.startingSeed)
        if args.seed_dir:
            seed_programs.extend(load_seeds_from_directory(Path(args.seed_dir)))
        seed_programs.extend(BUILTIN_TEMPLATES)

        # Deduplicate preserving order
        seen_progs: Set[str] = set()
        unique_seeds: List[str] = []
        for s in seed_programs:
            s_trimmed = s[: args.targetLen]
            if s_trimmed and s_trimmed not in seen_progs:
                seen_progs.add(s_trimmed)
                unique_seeds.append(s_trimmed)

        population: List[Individual] = []
        for i in range(args.population_size):
            if time.monotonic() - start_time >= args.timeout:
                break
            if i < len(unique_seeds):
                prog = unique_seeds[i]
            else:
                prog = generator.generate_program()
            ind = Individual(prog, harness=harness, generator=generator)
            population.append(ind)
            if (
                best_individual is None
                or (
                    ind.eval_result
                    and best_individual.eval_result
                    and ind.eval_result.individual_coverage.combined_score
                    > best_individual.eval_result.individual_coverage.combined_score
                )
            ):
                best_individual = ind

        if not population:
            return 0

        stagnant_generations = 0

        for generation in range(1, args.generations + 1):
            population.sort(key=lambda x: x.fitness)

            top = population[0]
            if (
                best_individual is None
                or (
                    top.eval_result
                    and best_individual.eval_result
                    and top.eval_result.individual_coverage.combined_score
                    > best_individual.eval_result.individual_coverage.combined_score
                )
            ):
                best_individual = top

            gen_new_cov = any(
                ind.eval_result and ind.eval_result.discovered_new_coverage
                for ind in population
            )
            if gen_new_cov:
                stagnant_generations = 0
            else:
                stagnant_generations += 1

            cum = harness.cumulative_metrics
            best_cov = (
                best_individual.eval_result.individual_coverage
                if best_individual and best_individual.eval_result
                else CoverageMetrics()
            )
            preview = top.program.replace("\n", " ")
            if len(preview) > 60:
                preview = preview[:57] + "..."

            print(
                f"Gen {generation:02d}/{args.generations:02d} | "
                f"CumCov: L={cum.lines_percent:5.1f}% ({cum.lines_covered}/{cum.lines_total}) "
                f"R={cum.regions_percent:5.1f}% B={cum.branches_percent:5.1f}% | "
                f"BestIndiv: {best_cov.lines_percent:5.1f}% | "
                f"Corpus: {len(harness.corpus)} | "
                f"Findings: {len(harness.findings)} | "
                f"Top: {preview}"
            )

            if top.fitness <= 0.0 and args.mode == "single":
                break
            if time.monotonic() - start_time >= args.timeout:
                print(f"Reached wall-clock timeout ({args.timeout:.1f}s).")
                break
            if generation == args.generations:
                break

            new_generation: List[Individual] = []

            # Elitism: keep top 15%
            elite_count = max(1, int(0.15 * len(population)))
            new_generation.extend(population[:elite_count])

            # If stagnating, inject fresh grammar-generated individuals & mutated corpus entries
            immigrant_count = (
                max(2, int(0.30 * args.population_size))
                if stagnant_generations >= args.stagnation_limit
                else max(1, int(0.10 * args.population_size))
            )
            for _ in range(immigrant_count):
                if time.monotonic() - start_time >= args.timeout:
                    break
                if harness.corpus and rng.random() < 0.5:
                    base_prog, _ = rng.choice(harness.corpus)
                    fresh_prog = generator.mutate(base_prog)
                else:
                    fresh_prog = generator.generate_program()
                new_generation.append(Individual(fresh_prog, harness=harness, generator=generator))

            # Mate from top 50% of population (plus corpus members) to fill remaining slots
            mating_pool = population[: max(2, len(population) // 2)]
            while len(new_generation) < args.population_size:
                if time.monotonic() - start_time >= args.timeout:
                    break
                parent1 = rng.choice(mating_pool)
                parent2 = rng.choice(mating_pool)
                child = parent1.mate(parent2)
                new_generation.append(child)

            population = new_generation

        cum = harness.cumulative_metrics
        best_prog = best_individual.program if best_individual else ""
        best_cov = (
            best_individual.eval_result.individual_coverage
            if best_individual and best_individual.eval_result
            else CoverageMetrics()
        )
        print("\n=== Fuzzing Summary ===")
        print(f"Mode                : {args.mode}")
        print(f"Total Evaluations   : {harness.runs}")
        print(
            f"Cumulative Coverage : Lines {cum.lines_percent:.2f}% ({cum.lines_covered}/{cum.lines_total}), "
            f"Regions {cum.regions_percent:.2f}% ({cum.regions_covered}/{cum.regions_total}), "
            f"Branches {cum.branches_percent:.2f}% ({cum.branches_covered}/{cum.branches_total})"
        )
        print(
            f"Best Single Input   : Lines {best_cov.lines_percent:.2f}%, "
            f"Regions {best_cov.regions_percent:.2f}% -> {best_prog!r}"
        )
        print(f"Corpus Entries      : {len(harness.corpus)}")
        print(f"Findings Discovered : {len(harness.findings)}")
        for idx, f in enumerate(harness.findings, start=1):
            print(f"  [{idx}] ({f.kind}) {f.description} | Program: {f.program!r}")

        if args.output_dir:
            out_path = Path(args.output_dir)
            save_outputs(out_path, harness, best_individual)
            print(f"Saved corpus and findings to: {out_path}")

        return 0
    finally:
        harness.close()


def main() -> None:
    parser = build_arg_parser()
    args = parser.parse_args()
    sys.exit(run_fuzzer(args))


if __name__ == "__main__":
    main()
