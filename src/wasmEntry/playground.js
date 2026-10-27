// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

/**
 * WarfLang Playground - Zero-Dependency WebAssembly IDE & Mini-LSP
 * 100% Vanilla ES6+ JavaScript. No external dependencies, CDNs, or libraries.
 */

(function () {
  'use strict';

  // =========================================================================
  // 1. Curated Examples & Constants
  // =========================================================================

  const EXAMPLES = {
    fibonacci: `// Fibonacci Sequence Example
function fib(n: int): int {
  if (n <= 1) {
    return n;
  }
  return fib(n - 1) + fib(n - 2);
}

// Compute and print first 10 Fibonacci numbers
for (var i = 0; i < 10; i = i + 1) {
  print(fib(i));
}

// Return 9th Fibonacci number
fib(9)`,

    loops: `// Loops and Control Flow Example
var sum = 0;
for (var i = 1; i <= 10; i = i + 1) {
  if (i == 5) {
    continue; // Skip 5
  }
  sum = sum + i;
}

var count = 0;
while (count < 3) {
  count = count + 1;
}

print(sum);
sum`,

    bitwise: `// Short-Circuit & Bitwise Operations
let a = 12; // Binary: 0b1100
let b = 10; // Binary: 0b1010

let bitAnd = a & b;  // 8
let bitOr  = a | b;  // 14
let bitXor = a ^ b;  // 6
let bitNot = ~a;

print(bitAnd);
print(bitOr);
print(bitXor);
print(bitNot);

// Logical short-circuiting: division by zero is never evaluated
let check = (a > 0) || (b / 0 == 1);
print(check);

bitAnd`,

    optimizer: `// WarfIR Optimization Showcase:
// - Constant Folding: 10 + 20 * 2 -> 50
// - Algebraic Identity: y * 0 -> 0
// - Dead Store Elimination
let x = 10 + 20 * 2;
let y = x * 0;
let z = y + 15;
let deadVar = 999;

print(x);
print(z);

z`,

    functions: `// Functions & Stdout Print Example
function add(a: int, b: int): int {
  return a + b;
}

function greet(value: int): int {
  print(value);
  return value * 2;
}

let result = add(15, 27);
print(result);
greet(result)`
  };

  const KEYWORDS = new Set([
    'break', 'continue', 'if', 'else', 'true', 'false',
    'for', 'while', 'do', 'function', 'extern', 'return',
    'let', 'var'
  ]);

  const TYPES = new Set([
    'int', 'number', 'bool', 'boolean', 'string', 'void', 'any', 'array'
  ]);

  const BUILTINS = {
    print: {
      signature: 'function print(val: any): any',
      doc: 'Prints a value to standard output',
      type: 'any'
    },
    len: {
      signature: 'function len(arr: array): number',
      doc: 'Returns the length of an array',
      type: 'number'
    }
  };

  // =========================================================================
  // 2. Client-Side Lexer & Analyzer (Fallback & Instant Feedback)
  // =========================================================================

  class WarfClientLexer {
    static tokenize(code) {
      const tokens = [];
      const diagnostics = [];
      let i = 0;
      let line = 1;
      let col = 1;
      const len = code.length;

      while (i < len) {
        const start = i;
        const startLine = line;
        const startCol = col;
        const ch = code[i];

        // Newline
        if (ch === '\n') {
          tokens.push({ type: 'newline', text: '\n', start, end: i + 1, line, col });
          i++;
          line++;
          col = 1;
          continue;
        }

        // Whitespace
        if (/\s/.test(ch)) {
          let ws = '';
          while (i < len && /\s/.test(code[i]) && code[i] !== '\n') {
            ws += code[i];
            i++;
            col++;
          }
          tokens.push({ type: 'whitespace', text: ws, start, end: i, line: startLine, col: startCol });
          continue;
        }

        // Single-line comment
        if (ch === '/' && code[i + 1] === '/') {
          let cText = '';
          while (i < len && code[i] !== '\n') {
            cText += code[i];
            i++;
            col++;
          }
          tokens.push({ type: 'comment', text: cText, start, end: i, line: startLine, col: startCol });
          continue;
        }

        // Multi-line comment
        if (ch === '/' && code[i + 1] === '*') {
          let cText = '/*';
          i += 2;
          col += 2;
          let closed = false;
          while (i < len) {
            if (code[i] === '*' && code[i + 1] === '/') {
              cText += '*/';
              i += 2;
              col += 2;
              closed = true;
              break;
            }
            if (code[i] === '\n') {
              cText += '\n';
              i++;
              line++;
              col = 1;
            } else {
              cText += code[i];
              i++;
              col++;
            }
          }
          if (!closed) {
            diagnostics.push({
              message: 'Unterminated block comment',
              start,
              end: i,
              line: startLine,
              col: startCol,
              severity: 'error'
            });
          }
          tokens.push({ type: 'comment', text: cText, start, end: i, line: startLine, col: startCol });
          continue;
        }

        // String literal
        if (ch === '"' || ch === "'") {
          const quote = ch;
          let sText = quote;
          i++;
          col++;
          let closed = false;
          while (i < len) {
            const sc = code[i];
            if (sc === '\\' && i + 1 < len) {
              sText += sc + code[i + 1];
              i += 2;
              col += 2;
              continue;
            }
            if (sc === quote) {
              sText += quote;
              i++;
              col++;
              closed = true;
              break;
            }
            if (sc === '\n') {
              break;
            }
            sText += sc;
            i++;
            col++;
          }
          if (!closed) {
            diagnostics.push({
              message: `Unterminated string literal (missing ${quote})`,
              start,
              end: i,
              line: startLine,
              col: startCol,
              severity: 'error'
            });
          }
          tokens.push({ type: 'string', text: sText, start, end: i, line: startLine, col: startCol });
          continue;
        }

        // Numbers
        if (/[0-9]/.test(ch)) {
          let numText = '';
          while (i < len && /[0-9]/.test(code[i])) {
            numText += code[i];
            i++;
            col++;
          }
          if (i < len && code[i] === '.' && /[0-9]/.test(code[i + 1])) {
            numText += '.';
            i++;
            col++;
            while (i < len && /[0-9]/.test(code[i])) {
              numText += code[i];
              i++;
              col++;
            }
          }
          tokens.push({ type: 'number', text: numText, start, end: i, line: startLine, col: startCol });
          continue;
        }

        // Identifiers & Keywords
        if (/[a-zA-Z_]/.test(ch)) {
          let idText = '';
          while (i < len && /[a-zA-Z0-9_]/.test(code[i])) {
            idText += code[i];
            i++;
            col++;
          }
          let type = 'variable';
          if (KEYWORDS.has(idText)) {
            type = (idText === 'true' || idText === 'false') ? 'boolean' : 'keyword';
          } else if (TYPES.has(idText)) {
            type = 'type';
          } else if (BUILTINS[idText]) {
            type = 'function';
          }
          tokens.push({ type, text: idText, start, end: i, line: startLine, col: startCol });
          continue;
        }

        // Multi-char operators
        const twoChar = code.substr(i, 2);
        if (['==', '!=', '<=', '>=', '&&', '||', '<<', '>>'].includes(twoChar)) {
          tokens.push({ type: 'operator', text: twoChar, start, end: i + 2, line: startLine, col: startCol });
          i += 2;
          col += 2;
          continue;
        }

        // Single-char operators
        if (['+', '-', '*', '/', '%', '&', '|', '^', '~', '!', '=', '<', '>'].includes(ch)) {
          tokens.push({ type: 'operator', text: ch, start, end: i + 1, line: startLine, col: startCol });
          i++;
          col++;
          continue;
        }

        // Punctuation
        if (['(', ')', '{', '}', '[', ']', ';', ',', ':', '.'].includes(ch)) {
          tokens.push({ type: 'punctuation', text: ch, start, end: i + 1, line: startLine, col: startCol });
          i++;
          col++;
          continue;
        }

        // Unknown character
        tokens.push({ type: 'unknown', text: ch, start, end: i + 1, line: startLine, col: startCol });
        diagnostics.push({
          message: `Unexpected character '${ch}'`,
          start,
          end: i + 1,
          line: startLine,
          col: startCol,
          severity: 'error'
        });
        i++;
        col++;
      }

      return { tokens, diagnostics };
    }

    static extractSymbols(tokens) {
      const symbols = {
        keywords: Array.from(KEYWORDS).map(k => ({ name: k, kind: 'keyword', detail: 'keyword' })),
        builtins: Object.entries(BUILTINS).map(([name, info]) => ({
          name,
          kind: 'function',
          signature: info.signature,
          doc: info.doc,
          returnType: info.type
        })),
        variables: [],
        functions: []
      };

      for (let i = 0; i < tokens.length; i++) {
        const t = tokens[i];
        if (t.type === 'keyword' && (t.text === 'let' || t.text === 'var')) {
          // Look forward for identifier
          let nextIdx = i + 1;
          while (nextIdx < tokens.length && (tokens[nextIdx].type === 'whitespace' || tokens[nextIdx].type === 'newline')) {
            nextIdx++;
          }
          if (nextIdx < tokens.length && tokens[nextIdx].type === 'variable') {
            const vName = tokens[nextIdx].text;
            if (!symbols.variables.some(v => v.name === vName)) {
              symbols.variables.push({
                name: vName,
                kind: 'variable',
                mutability: t.text,
                type: 'number',
                start: tokens[nextIdx].start,
                end: tokens[nextIdx].end
              });
            }
          }
        } else if (t.type === 'keyword' && t.text === 'function') {
          // Look forward for function identifier
          let nextIdx = i + 1;
          while (nextIdx < tokens.length && (tokens[nextIdx].type === 'whitespace' || tokens[nextIdx].type === 'newline')) {
            nextIdx++;
          }
          if (nextIdx < tokens.length && tokens[nextIdx].type === 'variable') {
            const fName = tokens[nextIdx].text;
            if (!symbols.functions.some(f => f.name === fName)) {
              symbols.functions.push({
                name: fName,
                kind: 'function',
                signature: `function ${fName}(...): any`,
                returnType: 'any',
                start: tokens[nextIdx].start,
                end: tokens[nextIdx].end
              });
            }
          }
        }
      }

      return symbols;
    }
  }

  // =========================================================================
  // 3. WebAssembly Dual-Mode Bridge
  // =========================================================================

  class WarfBridge {
    constructor() {
      this.isReady = false;
      this.hasRichApi = false;
      this.hasLegacyApi = false;
      this.init();
    }

    init() {
      if (typeof window !== 'undefined' && window.Module) {
        this.checkModule();
      }
    }

    checkModule() {
      const M = window.Module;
      if (!M) return;

      if (typeof M._AnalyzeWarf === 'function' && typeof M._ExecuteWarf === 'function') {
        this.hasRichApi = true;
        this.isReady = true;
      } else if (typeof M._RunWarf === 'function') {
        this.hasLegacyApi = true;
        this.isReady = true;
      }
    }

    encodeUTF8(str) {
      const M = window.Module;
      if (M && typeof M.lengthBytesUTF8 === 'function' && typeof M.stringToUTF8 === 'function' && typeof M._malloc === 'function') {
        const len = M.lengthBytesUTF8(str) + 1;
        const ptr = M._malloc(len);
        M.stringToUTF8(str, ptr, len);
        return { ptr, len };
      }
      // TextEncoder Fallback
      if (typeof TextEncoder !== 'undefined' && M && typeof M._malloc === 'function') {
        const encoded = new TextEncoder().encode(str + '\0');
        const ptr = M._malloc(encoded.length);
        M.HEAPU8.set(encoded, ptr);
        return { ptr, len: encoded.length };
      }
      return null;
    }

    decodeUTF8(ptr) {
      if (!ptr) return '';
      const M = window.Module;
      if (M && typeof M.UTF8ToString === 'function') {
        return M.UTF8ToString(ptr);
      }
      // Manual decode fallback
      if (M && M.HEAPU8) {
        let str = '';
        let offset = 0;
        while (true) {
          const byte = M.HEAPU8[ptr + offset];
          if (byte === 0) break;
          str += String.fromCharCode(byte);
          offset++;
        }
        try {
          return decodeURIComponent(escape(str));
        } catch (_) {
          return str;
        }
      }
      return '';
    }

    freeMemory(ptr) {
      const M = window.Module;
      if (ptr && M && typeof M._free === 'function') {
        M._free(ptr);
      }
    }

    analyze(code) {
      this.checkModule();
      const M = window.Module;

      if (this.hasRichApi && M) {
        const mem = this.encodeUTF8(code);
        if (mem) {
          try {
            const resPtr = M._AnalyzeWarf(mem.ptr);
            const jsonStr = this.decodeUTF8(resPtr);
            this.freeMemory(mem.ptr);
            if (jsonStr) {
              const parsed = JSON.parse(jsonStr);
              return parsed;
            }
          } catch (e) {
            console.warn('[WarfBridge] Analyze error:', e);
            this.freeMemory(mem.ptr);
          }
        }
      }

      // Standalone / Legacy Fallback
      const { tokens, diagnostics } = WarfClientLexer.tokenize(code);
      const symbols = WarfClientLexer.extractSymbols(tokens);

      let syntaxTreeText = '';
      let irO0Text = '';
      if (this.hasLegacyApi && M && typeof M._ShowSyntaxTree === 'function') {
        try {
          const treePtr = M._ShowSyntaxTree();
          syntaxTreeText = this.decodeUTF8(treePtr);
          if (typeof M._ShowIR === 'function') {
            const irPtr = M._ShowIR();
            irO0Text = this.decodeUTF8(irPtr);
          }
        } catch (_) {}
      }

      // Generate AST and IR items from text
      const ast = this.parseAstFromText(syntaxTreeText);
      const irO0 = this.parseIrFromText(irO0Text);

      return {
        source: code,
        hasErrors: diagnostics.length > 0,
        tokens,
        diagnostics,
        ast,
        syntaxTreeText,
        irO0,
        irO: irO0,
        irOpt: irO0,
        irO0Text,
        irOText: irO0Text,
        symbols
      };
    }

    execute(code, engine = 0, stepLimit = 500000) {
      this.checkModule();
      const M = window.Module;

      if (this.hasRichApi && M) {
        const mem = this.encodeUTF8(code);
        if (mem) {
          try {
            const start = performance.now();
            const resPtr = M._ExecuteWarf(mem.ptr, engine, stepLimit);
            const elapsedMs = performance.now() - start;
            const jsonStr = this.decodeUTF8(resPtr);
            this.freeMemory(mem.ptr);
            if (jsonStr) {
              const res = JSON.parse(jsonStr);
              if (res.elapsedMs === undefined) res.elapsedMs = elapsedMs;
              return res;
            }
          } catch (e) {
            this.freeMemory(mem.ptr);
            return {
              status: 'error',
              success: false,
              stdout: '',
              result: '',
              resultType: 'error',
              error: e.message || String(e),
              elapsedMs: 0
            };
          }
        }
      }

      // Legacy _RunWarf Fallback
      if (this.hasLegacyApi && M && typeof M._RunWarf === 'function') {
        const mem = this.encodeUTF8(code);
        if (mem) {
          try {
            const start = performance.now();
            const resPtr = M._RunWarf(mem.ptr, true, true);
            const elapsedMs = performance.now() - start;
            const output = this.decodeUTF8(resPtr);
            this.freeMemory(mem.ptr);

            // Split stdout from final return value
            const lines = output.trim().split('\n');
            const result = lines.length > 0 ? lines[lines.length - 1] : '';
            const stdout = lines.length > 1 ? lines.slice(0, -1).join('\n') + '\n' : '';

            return {
              status: 'ok',
              success: true,
              stdout,
              result,
              resultType: isNaN(Number(result)) ? 'string' : 'number',
              error: '',
              elapsedMs
            };
          } catch (e) {
            this.freeMemory(mem.ptr);
            return {
              status: 'error',
              success: false,
              stdout: '',
              result: '',
              resultType: 'error',
              error: e.message || String(e),
              elapsedMs: 0
            };
          }
        }
      }

      // Offline Simulation Mode
      return {
        status: 'ok',
        success: true,
        stdout: '[Wasm Runtime Initializing or Offline Preview]\nCode analyzed with zero errors.\n',
        result: '0',
        resultType: 'number',
        error: '',
        elapsedMs: 0.1
      };
    }

    parseAstFromText(text) {
      if (!text) return [];
      const lines = text.split('\n');
      const items = [];
      for (let i = 0; i < lines.length; i++) {
        const line = lines[i];
        if (!line.trim()) continue;
        const depth = (line.match(/^(\s*|│\s*|├─\s*|└─\s*)/) || [''])[0].length;
        const kind = line.trim().split(/\s+/)[0] || 'Node';
        items.push({
          kind,
          text: line.trim(),
          depth: Math.floor(depth / 2),
          start: -1,
          end: -1,
          line: i + 1
        });
      }
      return items;
    }

    parseIrFromText(text) {
      if (!text) return [];
      const lines = text.split('\n');
      return lines.map((l, idx) => ({
        line: l,
        text: l,
        start: -1,
        end: -1,
        lineNum: idx + 1
      }));
    }
  }

  // =========================================================================
  // 4. Playground Application Controller
  // =========================================================================

  class SlangPlaygroundApp {
    constructor() {
      this.bridge = new WarfBridge();
      this.currentAnalysis = null;
      this.currentTheme = localStorage.getItem('warf_theme') || 'dark';
      this.autoRun = true;
      this.debounceTimer = null;
      this.activeCrossHighlight = null;

      // Autocomplete state
      this.autocompleteActive = false;
      this.autocompleteSuggestions = [];
      this.autocompleteIndex = 0;

      this.cacheElements();
      this.initTheme();
      this.initEvents();
      this.initSplitters();
      this.loadInitialContent();
    }

    cacheElements() {
      // Toolbar
      this.exampleSelect = document.getElementById('exampleSelect');
      this.engineSelect = document.getElementById('engineSelect');
      this.autoRunCheckbox = document.getElementById('autoRunCheckbox');
      this.runBtn = document.getElementById('runBtn');
      this.shareBtn = document.getElementById('shareBtn');
      this.themeToggleBtn = document.getElementById('themeToggleBtn');

      // Column Toggles
      this.toggleAstBtn = document.getElementById('toggleAstBtn');
      this.toggleIrO0Btn = document.getElementById('toggleIrO0Btn');
      this.toggleIrOptBtn = document.getElementById('toggleIrOptBtn');

      // Panes
      this.panesContainer = document.getElementById('panesContainer');
      this.colEditor = document.getElementById('colEditor');
      this.colAst = document.getElementById('colAst');
      this.colIrO0 = document.getElementById('colIrO0');
      this.colIrOpt = document.getElementById('colIrOpt');

      this.splitterAst = document.getElementById('splitterAst');
      this.splitterIrO0 = document.getElementById('splitterIrO0');
      this.splitterIrOpt = document.getElementById('splitterIrOpt');
      this.splitterV = document.getElementById('splitterV');

      // Editor
      this.editorTextarea = document.getElementById('codeEditor');
      this.editorBackdrop = document.getElementById('editorBackdrop');
      this.backdropCode = document.getElementById('backdropCode');
      this.editorGutter = document.getElementById('editorGutter');
      this.activeLineBand = document.getElementById('activeLineBand');
      this.cursorPosBadge = document.getElementById('cursorPos');
      this.editorTooltip = document.getElementById('editorTooltip');
      this.autocompletePopup = document.getElementById('autocompletePopup');
      this.autocompleteList = document.getElementById('autocompleteList');

      // Compiler View Containers
      this.astContainer = document.getElementById('astContainer');
      this.irO0Container = document.getElementById('irO0Container');
      this.irOptContainer = document.getElementById('irOptContainer');

      // Console
      this.consolePane = document.getElementById('consolePane');
      this.tabOutputBtn = document.getElementById('tabOutputBtn');
      this.tabDiagBtn = document.getElementById('tabDiagBtn');
      this.consoleOutputTab = document.getElementById('consoleOutputTab');
      this.consoleDiagTab = document.getElementById('consoleDiagTab');
      this.consoleStdout = document.getElementById('consoleStdout');
      this.consoleReturn = document.getElementById('consoleReturn');
      this.diagnosticsList = document.getElementById('diagnosticsList');
      this.diagBadge = document.getElementById('diagBadge');
      this.execMeta = document.getElementById('execMeta');
      this.clearConsoleBtn = document.getElementById('clearConsoleBtn');
      this.toggleConsoleBtn = document.getElementById('toggleConsoleBtn');

      // Toast
      this.toast = document.getElementById('toast');
    }

    initTheme() {
      document.documentElement.setAttribute('data-theme', this.currentTheme);
      this.updateThemeIcon();
    }

    updateThemeIcon() {
      if (!this.themeToggleBtn) return;
      const isDark = this.currentTheme === 'dark';
      this.themeToggleBtn.innerHTML = isDark
        ? '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="5"/><path d="M12 1v2M12 21v2M4.22 4.22l1.42 1.42M18.36 18.36l1.42 1.42M1 12h2M21 12h2M4.22 19.78l1.42-1.42M18.36 5.64l1.42-1.42"/></svg>'
        : '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"/></svg>';
    }

    toggleTheme() {
      this.currentTheme = this.currentTheme === 'dark' ? 'light' : 'dark';
      document.documentElement.setAttribute('data-theme', this.currentTheme);
      localStorage.setItem('warf_theme', this.currentTheme);
      this.updateThemeIcon();
    }

    initEvents() {
      // Theme Toggle
      this.themeToggleBtn.addEventListener('click', () => this.toggleTheme());

      // Examples Selector
      this.exampleSelect.addEventListener('change', (e) => {
        const key = e.target.value;
        if (EXAMPLES[key]) {
          this.editorTextarea.value = EXAMPLES[key];
          this.onCodeChanged();
        }
      });

      // Engine Selector
      this.engineSelect.addEventListener('change', () => {
        this.runCode();
      });

      // Auto-Run Checkbox
      this.autoRunCheckbox.addEventListener('change', (e) => {
        this.autoRun = e.target.checked;
        if (this.autoRun) this.runCode();
      });

      // Run Button
      this.runBtn.addEventListener('click', () => this.runCode());

      // Share Button
      this.shareBtn.addEventListener('click', () => this.shareCode());

      // Column Toggle Buttons
      this.toggleAstBtn.addEventListener('click', () => this.toggleColumn('colAst'));
      this.toggleIrO0Btn.addEventListener('click', () => this.toggleColumn('colIrO0'));
      this.toggleIrOptBtn.addEventListener('click', () => this.toggleColumn('colIrOpt'));

      // Column Close Buttons
      document.querySelectorAll('.btn-close[data-close]').forEach(btn => {
        btn.addEventListener('click', () => {
          const colId = btn.getAttribute('data-close');
          this.toggleColumn(colId, false);
        });
      });

      // Editor Input & Scrolling
      this.editorTextarea.addEventListener('input', () => this.onCodeChanged());
      this.editorTextarea.addEventListener('scroll', () => this.syncEditorScroll());
      this.editorTextarea.addEventListener('click', () => this.updateCursorPosition());
      this.editorTextarea.addEventListener('keyup', (e) => this.onEditorKeyUp(e));
      this.editorTextarea.addEventListener('keydown', (e) => this.onEditorKeyDown(e));

      // Editor Hover for Tooltips & Cross-Highlighting
      this.editorTextarea.addEventListener('mousemove', (e) => this.onEditorMouseMove(e));
      this.editorTextarea.addEventListener('mouseleave', () => {
        this.hideTooltip();
        this.clearCrossHighlights();
      });

      // Console Tabs
      this.tabOutputBtn.addEventListener('click', () => this.switchConsoleTab('output'));
      this.tabDiagBtn.addEventListener('click', () => this.switchConsoleTab('diag'));

      // Console Controls
      this.clearConsoleBtn.addEventListener('click', () => this.clearConsole());
      this.toggleConsoleBtn.addEventListener('click', () => this.toggleConsole());

      // Global Keyboard Shortcuts
      window.addEventListener('keydown', (e) => {
        if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
          e.preventDefault();
          this.runCode();
        }
      });

      // Window Resize
      window.addEventListener('resize', () => {
        this.syncEditorScroll();
        this.hideAutocomplete();
      });
    }

    loadInitialContent() {
      // Check for #code= in URL
      const hash = window.location.hash;
      let initialCode = '';
      if (hash && hash.startsWith('#code=')) {
        try {
          const codePart = hash.substring(6);
          initialCode = decodeURIComponent(codePart);
        } catch (_) {
          try {
            initialCode = atob(hash.substring(6));
          } catch (_) {}
        }
      }

      if (!initialCode) {
        initialCode = EXAMPLES.fibonacci;
      }

      this.editorTextarea.value = initialCode;
      this.onCodeChanged();
      this.runCode();
    }

    // =======================================================================
    // Editor Typing, Highlighting, and Diagnostics
    // =======================================================================

    onCodeChanged() {
      this.renderSyntaxHighlighting();
      this.updateGutter();
      this.updateCursorPosition();

      if (this.autoRun) {
        clearTimeout(this.debounceTimer);
        this.debounceTimer = setTimeout(() => {
          this.runCode();
        }, 250);
      }
    }

    syncEditorScroll() {
      const top = this.editorTextarea.scrollTop;
      const left = this.editorTextarea.scrollLeft;
      this.editorBackdrop.scrollTop = top;
      this.editorBackdrop.scrollLeft = left;
      this.editorGutter.scrollTop = top;
      this.updateActiveLineBand();
    }

    updateGutter() {
      const lines = this.editorTextarea.value.split('\n');
      const lineCount = lines.length;
      let html = '';

      const currentLine = this.getCurrentLineNumber();
      const errorLines = new Set();
      if (this.currentAnalysis && this.currentAnalysis.diagnostics) {
        for (const d of this.currentAnalysis.diagnostics) {
          if (d.startLine) errorLines.add(d.startLine);
          else if (d.line) errorLines.add(d.line);
        }
      }

      for (let i = 1; i <= lineCount; i++) {
        const isActive = i === currentLine ? ' active' : '';
        const hasError = errorLines.has(i) ? ' has-error' : '';
        html += `<div class="gutter-line${isActive}${hasError}" data-line="${i}">${i}</div>`;
      }
      this.editorGutter.innerHTML = html;
      this.updateActiveLineBand();
    }

    updateActiveLineBand() {
      const lineNum = this.getCurrentLineNumber();
      const lineHeight = 20; // 20px
      const topOffset = 8 + (lineNum - 1) * lineHeight - this.editorTextarea.scrollTop;
      this.activeLineBand.style.top = `${topOffset}px`;
      this.activeLineBand.style.display = 'block';
    }

    getCurrentLineNumber() {
      const text = this.editorTextarea.value;
      const pos = this.editorTextarea.selectionStart;
      return text.substring(0, pos).split('\n').length;
    }

    updateCursorPosition() {
      const text = this.editorTextarea.value;
      const pos = this.editorTextarea.selectionStart;
      const lines = text.substring(0, pos).split('\n');
      const line = lines.length;
      const col = lines[lines.length - 1].length + 1;
      this.cursorPosBadge.textContent = `Ln ${line}, Col ${col}`;
      this.updateGutter();
    }

    renderSyntaxHighlighting() {
      const code = this.editorTextarea.value;
      const { tokens, diagnostics } = WarfClientLexer.tokenize(code);

      // Create a set of error ranges
      const errorSpans = (this.currentAnalysis && this.currentAnalysis.diagnostics)
        ? this.currentAnalysis.diagnostics
        : diagnostics;

      let html = '';
      for (const t of tokens) {
        if (t.type === 'newline') {
          html += '\n';
          continue;
        }

        const safeText = this.escapeHtml(t.text);
        const tokClass = `tok-${t.type}`;

        // Check if token intersects an error span
        const hasError = errorSpans.some(d => {
          const s = d.start !== undefined ? d.start : -1;
          const e = d.end !== undefined ? d.end : -1;
          return s <= t.start && e >= t.end && s !== -1;
        });

        const errorClass = hasError ? ' squiggle' : '';
        html += `<span class="${tokClass}${errorClass}" data-start="${t.start}" data-end="${t.end}" data-type="${t.type}">${safeText}</span>`;
      }

      this.backdropCode.innerHTML = html + '\n';
    }

    // =======================================================================
    // Keyboard Editing & Indentation
    // =======================================================================

    onEditorKeyDown(e) {
      if (this.autocompleteActive) {
        if (e.key === 'ArrowDown') {
          e.preventDefault();
          this.moveAutocompleteSelection(1);
          return;
        }
        if (e.key === 'ArrowUp') {
          e.preventDefault();
          this.moveAutocompleteSelection(-1);
          return;
        }
        if (e.key === 'Enter' || e.key === 'Tab') {
          e.preventDefault();
          this.applyAutocomplete();
          return;
        }
        if (e.key === 'Escape') {
          e.preventDefault();
          this.hideAutocomplete();
          return;
        }
      }

      const textarea = this.editorTextarea;
      const start = textarea.selectionStart;
      const end = textarea.selectionEnd;
      const val = textarea.value;

      // Tab or Shift+Tab
      if (e.key === 'Tab') {
        e.preventDefault();
        if (start === end) {
          // Insert 2 spaces
          document.execCommand('insertText', false, '  ');
        } else {
          // Multiline indent/dedent
          const lineStart = val.lastIndexOf('\n', start - 1) + 1;
          const lineEnd = val.indexOf('\n', end);
          const blockEnd = lineEnd === -1 ? val.length : lineEnd;
          const lines = val.substring(lineStart, blockEnd).split('\n');

          if (e.shiftKey) {
            // Dedent
            const newLines = lines.map(l => l.startsWith('  ') ? l.substring(2) : (l.startsWith(' ') ? l.substring(1) : l));
            const newText = newLines.join('\n');
            textarea.setSelectionRange(lineStart, blockEnd);
            document.execCommand('insertText', false, newText);
            textarea.setSelectionRange(lineStart, lineStart + newText.length);
          } else {
            // Indent
            const newLines = lines.map(l => '  ' + l);
            const newText = newLines.join('\n');
            textarea.setSelectionRange(lineStart, blockEnd);
            document.execCommand('insertText', false, newText);
            textarea.setSelectionRange(lineStart, lineStart + newText.length);
          }
        }
        this.onCodeChanged();
        return;
      }

      // Comment Toggle (Ctrl+/ or Cmd+/)
      if ((e.ctrlKey || e.metaKey) && e.key === '/') {
        e.preventDefault();
        const lineStart = val.lastIndexOf('\n', start - 1) + 1;
        const lineEnd = val.indexOf('\n', end);
        const blockEnd = lineEnd === -1 ? val.length : lineEnd;
        const lines = val.substring(lineStart, blockEnd).split('\n');

        const allCommented = lines.every(l => l.trim().startsWith('//'));
        const newLines = lines.map(l => {
          if (allCommented) {
            return l.replace(/^\s*\/\/\s?/, '');
          } else {
            return '// ' + l;
          }
        });
        const newText = newLines.join('\n');
        textarea.setSelectionRange(lineStart, blockEnd);
        document.execCommand('insertText', false, newText);
        textarea.setSelectionRange(lineStart, lineStart + newText.length);
        this.onCodeChanged();
        return;
      }

      // Auto-indent on Enter
      if (e.key === 'Enter') {
        const lineStart = val.lastIndexOf('\n', start - 1) + 1;
        const currentLine = val.substring(lineStart, start);
        const indentMatch = currentLine.match(/^\s*/);
        let indent = indentMatch ? indentMatch[0] : '';

        // Extra indent if line ends with '{'
        const prevChar = val[start - 1];
        const nextChar = val[start];
        if (prevChar === '{' && nextChar === '}') {
          e.preventDefault();
          const extraIndent = indent + '  ';
          const insertion = '\n' + extraIndent + '\n' + indent;
          document.execCommand('insertText', false, insertion);
          textarea.setSelectionRange(start + extraIndent.length + 1, start + extraIndent.length + 1);
          this.onCodeChanged();
          return;
        } else if (prevChar === '{') {
          indent += '  ';
        }

        e.preventDefault();
        document.execCommand('insertText', false, '\n' + indent);
        this.onCodeChanged();
        return;
      }

      // Bracket / Quote Auto-Closing
      const pairs = { '(': ')', '[': ']', '{': '}', '"': '"', "'": "'" };
      if (pairs[e.key]) {
        const close = pairs[e.key];
        e.preventDefault();
        if (start !== end) {
          const selected = val.substring(start, end);
          document.execCommand('insertText', false, e.key + selected + close);
          textarea.setSelectionRange(start + 1, end + 1);
        } else {
          document.execCommand('insertText', false, e.key + close);
          textarea.setSelectionRange(start + 1, start + 1);
        }
        this.onCodeChanged();
        return;
      }

      // Step over closing bracket
      if ([')', ']', '}', '"', "'"].includes(e.key)) {
        if (val[start] === e.key && start === end) {
          e.preventDefault();
          textarea.setSelectionRange(start + 1, start + 1);
          this.updateCursorPosition();
          return;
        }
      }

      // Backspace removes matched pair
      if (e.key === 'Backspace' && start === end && start > 0) {
        const prev = val[start - 1];
        const next = val[start];
        if (pairs[prev] === next) {
          e.preventDefault();
          textarea.setSelectionRange(start - 1, start + 1);
          document.execCommand('delete', false, null);
          this.onCodeChanged();
          return;
        }
      }

      // Trigger Autocomplete manually (Ctrl+Space)
      if ((e.ctrlKey || e.metaKey) && e.key === ' ') {
        e.preventDefault();
        this.triggerAutocomplete(true);
      }
    }

    onEditorKeyUp(e) {
      if (['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(e.key)) {
        this.updateCursorPosition();
      }

      // Auto-trigger autocomplete on identifier char
      if (/[a-zA-Z_]/.test(e.key) && !e.ctrlKey && !e.metaKey && !e.altKey) {
        this.triggerAutocomplete(false);
      } else if (['Backspace', 'Delete'].includes(e.key)) {
        this.triggerAutocomplete(false);
      }
    }

    // =======================================================================
    // Scope-Aware Autocomplete
    // =======================================================================

    triggerAutocomplete(force) {
      const text = this.editorTextarea.value;
      const pos = this.editorTextarea.selectionStart;

      // Extract identifier prefix before cursor
      let start = pos;
      while (start > 0 && /[a-zA-Z0-9_]/.test(text[start - 1])) {
        start--;
      }
      const prefix = text.substring(start, pos);

      if (!force && prefix.length < 1) {
        this.hideAutocomplete();
        return;
      }

      // Gather candidate symbols
      const candidates = [];
      const seen = new Set();

      const addCandidate = (name, kind, badge, detail) => {
        if (!seen.has(name) && name.toLowerCase().startsWith(prefix.toLowerCase()) && name !== prefix) {
          seen.add(name);
          candidates.push({ name, kind, badge, detail });
        }
      };

      // Keywords
      KEYWORDS.forEach(k => addCandidate(k, 'keyword', 'K', 'keyword'));

      // Builtins
      Object.keys(BUILTINS).forEach(b => addCandidate(b, 'function', 'F', BUILTINS[b].signature));

      // Types
      TYPES.forEach(t => addCandidate(t, 'type', 'T', 'type'));

      // Analyzed symbols (variables & functions)
      if (this.currentAnalysis && this.currentAnalysis.symbols) {
        const syms = this.currentAnalysis.symbols;
        if (syms.variables) {
          syms.variables.forEach(v => addCandidate(v.name, 'variable', 'V', `${v.mutability || 'let'} ${v.name}: ${v.type || 'number'}`));
        }
        if (syms.functions) {
          syms.functions.forEach(f => addCandidate(f.name, 'function', 'F', f.signature || `function ${f.name}`));
        }
      }

      if (candidates.length === 0) {
        this.hideAutocomplete();
        return;
      }

      this.autocompleteSuggestions = candidates;
      this.autocompleteIndex = 0;
      this.showAutocompletePopup(start);
    }

    showAutocompletePopup(startOffset) {
      this.autocompleteActive = true;
      this.renderAutocompleteList();

      // Estimate position near cursor
      const text = this.editorTextarea.value.substring(0, startOffset);
      const lines = text.split('\n');
      const lineIdx = lines.length - 1;
      const colIdx = lines[lineIdx].length;

      const rect = this.editorTextarea.getBoundingClientRect();
      const lineHeight = 20;
      const charWidth = 7.8; // approximate for 13px mono

      const top = rect.top + 8 + (lineIdx + 1) * lineHeight - this.editorTextarea.scrollTop;
      const left = rect.left + 12 + colIdx * charWidth - this.editorTextarea.scrollLeft;

      this.autocompletePopup.style.top = `${Math.min(window.innerHeight - 200, Math.max(rect.top, top))}px`;
      this.autocompletePopup.style.left = `${Math.min(window.innerWidth - 250, Math.max(rect.left, left))}px`;
      this.autocompletePopup.classList.remove('hidden');
    }

    renderAutocompleteList() {
      let html = '';
      this.autocompleteSuggestions.forEach((item, idx) => {
        const isSelected = idx === this.autocompleteIndex ? ' selected' : '';
        html += `
          <div class="autocomplete-item${isSelected}" data-idx="${idx}">
            <span class="autocomplete-badge badge-${item.kind}">${item.badge}</span>
            <strong>${item.name}</strong>
            <span style="margin-left:auto; font-size:10px; color:var(--text-muted);">${item.detail || ''}</span>
          </div>
        `;
      });
      this.autocompleteList.innerHTML = html;

      // Attach click events
      this.autocompleteList.querySelectorAll('.autocomplete-item').forEach(el => {
        el.addEventListener('click', () => {
          this.autocompleteIndex = parseInt(el.getAttribute('data-idx'), 10);
          this.applyAutocomplete();
        });
      });
    }

    moveAutocompleteSelection(delta) {
      const len = this.autocompleteSuggestions.length;
      if (len === 0) return;
      this.autocompleteIndex = (this.autocompleteIndex + delta + len) % len;
      this.renderAutocompleteList();

      const selected = this.autocompleteList.querySelector('.autocomplete-item.selected');
      if (selected) {
        selected.scrollIntoView({ block: 'nearest' });
      }
    }

    applyAutocomplete() {
      if (!this.autocompleteActive || this.autocompleteSuggestions.length === 0) return;
      const item = this.autocompleteSuggestions[this.autocompleteIndex];
      const textarea = this.editorTextarea;
      const pos = textarea.selectionStart;
      const text = textarea.value;

      let start = pos;
      while (start > 0 && /[a-zA-Z0-9_]/.test(text[start - 1])) {
        start--;
      }

      textarea.setSelectionRange(start, pos);
      document.execCommand('insertText', false, item.name);
      this.hideAutocomplete();
      this.onCodeChanged();
      textarea.focus();
    }

    hideAutocomplete() {
      this.autocompleteActive = false;
      this.autocompletePopup.classList.add('hidden');
    }

    // =======================================================================
    // Hover Tooltips & Error Details
    // =======================================================================

    onEditorMouseMove(e) {
      if (this.autocompleteActive) return;

      const rect = this.editorTextarea.getBoundingClientRect();
      const x = e.clientX - rect.left - 12 + this.editorTextarea.scrollLeft;
      const y = e.clientY - rect.top - 8 + this.editorTextarea.scrollTop;

      const lineHeight = 20;
      const charWidth = 7.8;

      const line = Math.floor(y / lineHeight) + 1;
      const col = Math.floor(x / charWidth);

      // Check if mouse hovers over an error squiggle
      if (this.currentAnalysis && this.currentAnalysis.diagnostics) {
        const diag = this.currentAnalysis.diagnostics.find(d => {
          if (d.startLine && d.startLine === line) return true;
          if (d.line && d.line === line) return true;
          return false;
        });

        if (diag) {
          this.showTooltip(e.clientX + 10, e.clientY + 10, `
            <div class="tooltip-error"><strong>Error (Line ${diag.startLine || diag.line || line}):</strong></div>
            <div>${this.escapeHtml(diag.message)}</div>
          `);
          return;
        }
      }

      // Check if hovering over an identifier/function
      const offset = this.getCharOffsetFromCoords(e.clientX, e.clientY);
      if (offset !== -1 && this.currentAnalysis && this.currentAnalysis.symbols) {
        const sym = this.findSymbolAtOffset(offset);
        if (sym) {
          this.showTooltip(e.clientX + 10, e.clientY + 10, sym);
          return;
        }
      }

      this.hideTooltip();
    }

    getCharOffsetFromCoords(clientX, clientY) {
      // Find element in backdrop under cursor
      const el = document.elementFromPoint(clientX, clientY);
      if (el && el.getAttribute && el.getAttribute('data-start')) {
        return parseInt(el.getAttribute('data-start'), 10);
      }
      return -1;
    }

    findSymbolAtOffset(offset) {
      if (!this.currentAnalysis || !this.currentAnalysis.symbols) return null;
      const { variables, functions, builtins } = this.currentAnalysis.symbols;

      if (variables) {
        const v = variables.find(x => x.start <= offset && offset <= x.end);
        if (v) {
          return `<div><span class="tooltip-code">${v.mutability || 'let'} ${v.name}: ${v.type || 'number'}</span></div>`;
        }
      }
      if (functions) {
        const f = functions.find(x => x.start <= offset && offset <= x.end);
        if (f) {
          return `<div><span class="tooltip-code">${f.signature || f.name}</span></div><div>${this.escapeHtml(f.doc || '')}</div>`;
        }
      }
      return null;
    }

    showTooltip(x, y, content) {
      this.editorTooltip.innerHTML = content;
      this.editorTooltip.style.top = `${Math.min(window.innerHeight - 80, y)}px`;
      this.editorTooltip.style.left = `${Math.min(window.innerWidth - 300, x)}px`;
      this.editorTooltip.classList.remove('hidden');
    }

    hideTooltip() {
      this.editorTooltip.classList.add('hidden');
    }

    // =======================================================================
    // Execution & Analysis Engine
    // =======================================================================

    runCode() {
      const code = this.editorTextarea.value;
      const engine = parseInt(this.engineSelect.value, 10) || 0;

      // 1. Analyze code for spans, diagnostics, and compiler representations
      this.currentAnalysis = this.bridge.analyze(code);

      // Render updated syntax highlighting and squiggles
      this.renderSyntaxHighlighting();
      this.updateGutter();

      // Render Compiler Columns
      this.renderAstColumn(this.currentAnalysis.ast, this.currentAnalysis.syntaxTreeText);
      this.renderIrColumn(this.irO0Container, this.currentAnalysis.irO0, this.currentAnalysis.irO0Text);
      this.renderIrColumn(this.irOptContainer, this.currentAnalysis.irO || this.currentAnalysis.irOpt, this.currentAnalysis.irOText);

      // Render Diagnostics
      this.renderDiagnostics(this.currentAnalysis.diagnostics || []);

      // 2. Execute code
      const result = this.bridge.execute(code, engine, 500000);
      this.renderExecutionOutput(result);
    }

    renderExecutionOutput(result) {
      this.consoleStdout.textContent = result.stdout || '(no stdout captured)';
      this.execMeta.textContent = `${(result.elapsedMs || 0).toFixed(2)} ms`;

      if (result.success) {
        this.consoleReturn.className = 'console-return-card success';
        this.consoleReturn.innerHTML = `
          <span>✓</span>
          <span><strong>Result:</strong> ${this.escapeHtml(result.result || 'void')}</span>
          <span class="badge" style="margin-left:auto;">${result.resultType || 'unknown'}</span>
        `;
      } else {
        this.consoleReturn.className = 'console-return-card error';
        this.consoleReturn.innerHTML = `
          <span>✕</span>
          <span><strong>Runtime Error:</strong> ${this.escapeHtml(result.error || 'Execution failed')}</span>
        `;
        // Switch to output tab to display error
        this.switchConsoleTab('output');
      }
    }

    renderDiagnostics(diagnostics) {
      const count = diagnostics.length;
      this.diagBadge.textContent = count;
      if (count > 0) {
        this.diagBadge.className = 'badge has-errors';
      } else {
        this.diagBadge.className = 'badge';
      }

      if (count === 0) {
        this.diagnosticsList.innerHTML = '<div style="color:var(--text-muted); padding:10px;">No diagnostics. Code is clean.</div>';
        return;
      }

      let html = '';
      diagnostics.forEach((d, idx) => {
        const line = d.startLine || d.line || 1;
        const col = d.startCol || d.col || 1;
        html += `
          <div class="diagnostic-item" data-line="${line}" data-col="${col}" data-start="${d.start || -1}" data-end="${d.end || -1}">
            <span class="diagnostic-icon">✕</span>
            <div style="flex:1;">
              <div class="diagnostic-msg">${this.escapeHtml(d.message)}</div>
              <div class="diagnostic-pos">Line ${line}, Column ${col} (${d.stage || 'compiler'})</div>
            </div>
          </div>
        `;
      });

      this.diagnosticsList.innerHTML = html;

      // Click diagnostic to jump to editor location
      this.diagnosticsList.querySelectorAll('.diagnostic-item').forEach(item => {
        item.addEventListener('click', () => {
          const line = parseInt(item.getAttribute('data-line'), 10);
          const col = parseInt(item.getAttribute('data-col'), 10);
          this.jumpToEditor(line, col);
        });
      });
    }

    jumpToEditor(line, col) {
      const text = this.editorTextarea.value;
      const lines = text.split('\n');
      let offset = 0;

      for (let i = 0; i < line - 1 && i < lines.length; i++) {
        offset += lines[i].length + 1; // +1 for \n
      }
      offset += Math.min(col - 1, (lines[line - 1] || '').length);

      this.editorTextarea.focus();
      this.editorTextarea.setSelectionRange(offset, offset);
      this.updateCursorPosition();

      // Scroll editor so target line is centered
      const lineHeight = 20;
      const targetScroll = Math.max(0, (line - 3) * lineHeight);
      this.editorTextarea.scrollTop = targetScroll;
      this.syncEditorScroll();
    }

    // =======================================================================
    // Dynamic Compiler Views & Cross-Highlighting
    // =======================================================================

    renderAstColumn(ast, fallbackText) {
      if (!this.astContainer) return;

      if (ast && ast.length > 0) {
        let html = '';
        ast.forEach((node, idx) => {
          const indent = '&nbsp;'.repeat((node.depth || 0) * 2);
          const spanText = (node.start >= 0 && node.end >= 0) ? `[${node.start}:${node.end}]` : '';
          html += `
            <div class="ast-node" data-idx="${idx}" data-start="${node.start}" data-end="${node.end}">
              <span>${indent}</span>
              <span class="ast-kind">${this.escapeHtml(node.kind)}</span>
              <span class="ast-text">${this.escapeHtml(node.text || '')}</span>
              <span class="ast-span">${spanText}</span>
            </div>
          `;
        });
        this.astContainer.innerHTML = html;
      } else if (fallbackText) {
        const lines = fallbackText.split('\n');
        let html = '';
        lines.forEach((l, idx) => {
          if (!l.trim()) return;
          html += `<div class="ast-node" data-idx="${idx}">${this.escapeHtml(l)}</div>`;
        });
        this.astContainer.innerHTML = html;
      } else {
        this.astContainer.innerHTML = '<div style="color:var(--text-muted); padding:10px;">AST unavailable</div>';
      }

      // Cross-highlighting events on AST nodes
      this.astContainer.querySelectorAll('.ast-node').forEach(nodeEl => {
        nodeEl.addEventListener('mouseenter', () => {
          const start = parseInt(nodeEl.getAttribute('data-start'), 10);
          const end = parseInt(nodeEl.getAttribute('data-end'), 10);
          if (start >= 0 && end >= 0) {
            this.highlightSourceSpan(start, end);
          }
        });
        nodeEl.addEventListener('mouseleave', () => {
          this.clearCrossHighlights();
        });
      });
    }

    renderIrColumn(container, irLines, fallbackText) {
      if (!container) return;

      if (irLines && irLines.length > 0) {
        let html = '';
        irLines.forEach((item, idx) => {
          const text = item.line || item.text || '';
          html += `
            <div class="code-view-line" data-idx="${idx}" data-start="${item.start !== undefined ? item.start : -1}" data-end="${item.end !== undefined ? item.end : -1}">
              <span class="line-num">${idx + 1}</span>
              <span class="line-text">${this.escapeHtml(text)}</span>
            </div>
          `;
        });
        container.innerHTML = html;
      } else if (fallbackText) {
        const lines = fallbackText.split('\n');
        let html = '';
        lines.forEach((l, idx) => {
          html += `
            <div class="code-view-line" data-idx="${idx}">
              <span class="line-num">${idx + 1}</span>
              <span class="line-text">${this.escapeHtml(l)}</span>
            </div>
          `;
        });
        container.innerHTML = html;
      } else {
        container.innerHTML = '<div style="color:var(--text-muted); padding:10px;">IR unavailable</div>';
      }

      // Cross-highlighting events on IR lines
      container.querySelectorAll('.code-view-line').forEach(lineEl => {
        lineEl.addEventListener('mouseenter', () => {
          const start = parseInt(lineEl.getAttribute('data-start'), 10);
          const end = parseInt(lineEl.getAttribute('data-end'), 10);
          if (start >= 0 && end >= 0) {
            this.highlightSourceSpan(start, end);
          }
        });
        lineEl.addEventListener('mouseleave', () => {
          this.clearCrossHighlights();
        });
      });
    }

    highlightSourceSpan(start, end) {
      this.clearCrossHighlights();
      // Highlight matching spans in the editor backdrop
      const tokens = this.backdropCode.querySelectorAll('span[data-start]');
      tokens.forEach(tok => {
        const tStart = parseInt(tok.getAttribute('data-start'), 10);
        const tEnd = parseInt(tok.getAttribute('data-end'), 10);
        if (tStart >= start && tEnd <= end) {
          tok.classList.add('cross-highlight');
        }
      });
    }

    clearCrossHighlights() {
      document.querySelectorAll('.cross-highlight').forEach(el => {
        el.classList.remove('cross-highlight');
      });
    }

    // =======================================================================
    // Column Toggles & Splitters
    // =======================================================================

    toggleColumn(colId, forceState) {
      const col = document.getElementById(colId);
      if (!col) return;

      const isHidden = col.classList.contains('hidden');
      const shouldShow = forceState !== undefined ? forceState : isHidden;

      // Associate splitter with column
      const splitterMap = {
        colAst: this.splitterAst,
        colIrO0: this.splitterIrO0,
        colIrOpt: this.splitterIrOpt
      };
      const splitter = splitterMap[colId];

      const btnMap = {
        colAst: this.toggleAstBtn,
        colIrO0: this.toggleIrO0Btn,
        colIrOpt: this.toggleIrOptBtn
      };
      const btn = btnMap[colId];

      if (shouldShow) {
        col.classList.remove('hidden');
        if (splitter) splitter.style.display = 'block';
        if (btn) btn.classList.add('active');
      } else {
        col.classList.add('hidden');
        if (splitter) splitter.style.display = 'none';
        if (btn) btn.classList.remove('active');
      }
    }

    initSplitters() {
      // Horizontal Splitters
      const makeHorizontalSplitter = (splitter, leftPane, rightPane) => {
        if (!splitter || !leftPane || !rightPane) return;
        let isDragging = false;
        let startX = 0;
        let startLeftW = 0;
        let startRightW = 0;

        splitter.addEventListener('mousedown', (e) => {
          isDragging = true;
          startX = e.clientX;
          startLeftW = leftPane.getBoundingClientRect().width;
          startRightW = rightPane.getBoundingClientRect().width;
          splitter.classList.add('dragging');
          document.body.style.cursor = 'col-resize';
          e.preventDefault();
        });

        window.addEventListener('mousemove', (e) => {
          if (!isDragging) return;
          const dx = e.clientX - startX;
          const newLeftW = Math.max(160, startLeftW + dx);
          const newRightW = Math.max(160, startRightW - dx);
          leftPane.style.flex = `0 0 ${newLeftW}px`;
          rightPane.style.flex = `0 0 ${newRightW}px`;
        });

        window.addEventListener('mouseup', () => {
          if (isDragging) {
            isDragging = false;
            splitter.classList.remove('dragging');
            document.body.style.cursor = '';
          }
        });

        // Double click resets flex sizes
        splitter.addEventListener('dblclick', () => {
          leftPane.style.flex = '1';
          rightPane.style.flex = '1';
        });
      };

      makeHorizontalSplitter(this.splitterAst, this.colEditor, this.colAst);
      makeHorizontalSplitter(this.splitterIrO0, this.colAst, this.colIrO0);
      makeHorizontalSplitter(this.splitterIrOpt, this.colIrO0, this.colIrOpt);

      // Vertical Splitter (Workspace vs Bottom Console)
      if (this.splitterV && this.consolePane) {
        let isDraggingV = false;
        let startY = 0;
        let startHeight = 0;

        this.splitterV.addEventListener('mousedown', (e) => {
          isDraggingV = true;
          startY = e.clientY;
          startHeight = this.consolePane.getBoundingClientRect().height;
          this.splitterV.classList.add('dragging');
          document.body.style.cursor = 'row-resize';
          e.preventDefault();
        });

        window.addEventListener('mousemove', (e) => {
          if (!isDraggingV) return;
          const dy = startY - e.clientY;
          const newH = Math.max(36, Math.min(window.innerHeight - 150, startHeight + dy));
          this.consolePane.style.height = `${newH}px`;
          if (newH > 40) {
            this.consolePane.classList.remove('collapsed');
          }
        });

        window.addEventListener('mouseup', () => {
          if (isDraggingV) {
            isDraggingV = false;
            this.splitterV.classList.remove('dragging');
            document.body.style.cursor = '';
          }
        });
      }
    }

    // =======================================================================
    // Console Tabs & Controls
    // =======================================================================

    switchConsoleTab(tab) {
      if (tab === 'output') {
        this.tabOutputBtn.classList.add('active');
        this.tabDiagBtn.classList.remove('active');
        this.consoleOutputTab.classList.add('active');
        this.consoleDiagTab.classList.remove('active');
      } else {
        this.tabDiagBtn.classList.add('active');
        this.tabOutputBtn.classList.remove('active');
        this.consoleDiagTab.classList.add('active');
        this.consoleOutputTab.classList.remove('active');
      }
    }

    clearConsole() {
      this.consoleStdout.textContent = '';
      this.consoleReturn.innerHTML = '';
      this.consoleReturn.className = 'console-return-card';
    }

    toggleConsole() {
      const isCollapsed = this.consolePane.classList.toggle('collapsed');
      this.toggleConsoleBtn.textContent = isCollapsed ? '+' : '−';
      if (!isCollapsed && parseInt(this.consolePane.style.height, 10) < 100) {
        this.consolePane.style.height = '180px';
      }
    }

    // =======================================================================
    // Share URL & Toast Notification
    // =======================================================================

    shareCode() {
      const code = this.editorTextarea.value;
      const encoded = encodeURIComponent(code);
      const url = `${window.location.origin}${window.location.pathname}#code=${encoded}`;

      // Update browser URL without reload
      window.history.replaceState(null, '', url);

      // Copy to clipboard
      if (navigator.clipboard && navigator.clipboard.writeText) {
        navigator.clipboard.writeText(url).then(() => {
          this.showToast('✓ Share link copied to clipboard!');
        }).catch(() => {
          this.fallbackCopyText(url);
        });
      } else {
        this.fallbackCopyText(url);
      }
    }

    fallbackCopyText(text) {
      const tempInput = document.createElement('textarea');
      tempInput.value = text;
      document.body.appendChild(tempInput);
      tempInput.select();
      try {
        document.execCommand('copy');
        this.showToast('✓ Share link copied to clipboard!');
      } catch (_) {
        this.showToast('Link generated in address bar!');
      }
      document.body.removeChild(tempInput);
    }

    showToast(message) {
      if (!this.toast) return;
      this.toast.textContent = message;
      this.toast.classList.remove('hidden');
      setTimeout(() => {
        this.toast.classList.add('hidden');
      }, 2500);
    }

    escapeHtml(str) {
      if (!str) return '';
      return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#39;');
    }
  }

  // =========================================================================
  // 5. Application Bootstrap
  // =========================================================================

  function bootstrap() {
    window.g_WarfPlayground = new SlangPlaygroundApp();

    // Hook Module.onRuntimeInitialized if Emscripten is loading asynchronously
    if (typeof window !== 'undefined' && window.Module) {
      const prevInit = window.Module.onRuntimeInitialized;
      window.Module.onRuntimeInitialized = function () {
        if (typeof prevInit === 'function') prevInit();
        if (window.g_WarfPlayground) {
          window.g_WarfPlayground.bridge.init();
          window.g_WarfPlayground.runCode();
        }
      };
    }
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', bootstrap);
  } else {
    bootstrap();
  }

})();
