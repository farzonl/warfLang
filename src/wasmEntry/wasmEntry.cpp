// Copyright (c) 2023 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdbool.h>
#include <string.h>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(__EMSCRIPTEN__) || defined(Emscripten)
#include <emscripten.h>
#else
#ifndef EMSCRIPTEN_KEEPALIVE
#define EMSCRIPTEN_KEEPALIVE
#endif
#endif

#include "Binding/Binder.h"
#include "Binding/BoundBlockStatementNode.h"
#include "Binding/BoundDoWhileStatementNode.h"
#include "Binding/BoundExpressionStatementNode.h"
#include "Binding/BoundForStatementNode.h"
#include "Binding/BoundFunctionDeclarationNode.h"
#include "Binding/BoundIfStatementNode.h"
#include "Binding/BoundVariableDeclarationNode.h"
#include "Binding/BoundWhileStatementNode.h"
#include "Binding/ValueType.h"
#include "CodeAnalysis/Evaluator.h"
#include "IR/Ir.h"
#include "IR/IrOptimizer.h"
#include "Symbol/FunctionSymbol.h"
#include "Symbol/SymbolTableMgr.h"
#include "Symbol/TextSpan.h"
#include "Symbol/VariableSymbol.h"
#include "Syntax/CompilationUnitSyntaxNode.h"
#include "Syntax/FunctionDeclarationSyntaxNode.h"
#include "Syntax/Lexer.h"
#include "Syntax/SyntaxKind.h"
#include "Syntax/SyntaxNode.h"
#include "Syntax/SyntaxToken.h"
#include "Syntax/SyntaxTree.h"
#include "Syntax/VariableDeclarationSyntaxNode.h"

// ---------------------------------------------------------------------------
// RAII Cout Redirection Guard
// ---------------------------------------------------------------------------
namespace {
class CoutRedirect {
public:
  CoutRedirect() : mOldBuf(std::cout.rdbuf(mBuffer.rdbuf())) {}
  ~CoutRedirect() { std::cout.rdbuf(mOldBuf); }
  std::string getString() const { return mBuffer.str(); }

  CoutRedirect(const CoutRedirect &) = delete;
  CoutRedirect &operator=(const CoutRedirect &) = delete;

private:
  std::stringstream mBuffer;
  std::streambuf *mOldBuf;
};

// ---------------------------------------------------------------------------
// SFINAE Helpers for Step Budget and Source Spans
// ---------------------------------------------------------------------------
template <typename T, typename Limit, typename = void>
struct HasSetStepLimit : std::false_type {};

template <typename T, typename Limit>
struct HasSetStepLimit<T, Limit,
                       std::void_t<decltype(std::declval<T>().SetStepLimit(
                           std::declval<Limit>()))>> : std::true_type {};

template <typename T> void ApplyStepLimit(T &target, int stepLimit) {
  if (stepLimit <= 0) {
    return;
  }
  if constexpr (HasSetStepLimit<T, int>::value) {
    target.SetStepLimit(stepLimit);
  } else if constexpr (HasSetStepLimit<T, uint64_t>::value) {
    target.SetStepLimit(static_cast<uint64_t>(stepLimit));
  } else if constexpr (HasSetStepLimit<T, int64_t>::value) {
    target.SetStepLimit(static_cast<int64_t>(stepLimit));
  } else if constexpr (HasSetStepLimit<T, size_t>::value) {
    target.SetStepLimit(static_cast<size_t>(stepLimit));
  }
}

template <typename T, typename = void> struct HasSpanField : std::false_type {};

template <typename T>
struct HasSpanField<T, std::void_t<decltype(std::declval<T>().span)>>
    : std::true_type {};

template <typename Inst> TextSpan GetInstructionSpan(const Inst &inst) {
  if constexpr (HasSpanField<Inst>::value) {
    return inst.span;
  } else {
    return TextSpan(-1, -1);
  }
}

// ---------------------------------------------------------------------------
// JSON Serialization Helpers
// ---------------------------------------------------------------------------
std::string EscapeJsonString(const std::string &str) {
  std::ostringstream ss;
  size_t i = 0;
  while (i < str.size()) {
    unsigned char c = static_cast<unsigned char>(str[i]);
    if (c == '"') {
      ss << "\\\"";
      i++;
    } else if (c == '\\') {
      ss << "\\\\";
      i++;
    } else if (c == '\b') {
      ss << "\\b";
      i++;
    } else if (c == '\f') {
      ss << "\\f";
      i++;
    } else if (c == '\n') {
      ss << "\\n";
      i++;
    } else if (c == '\r') {
      ss << "\\r";
      i++;
    } else if (c == '\t') {
      ss << "\\t";
      i++;
    } else if (c < 0x20) {
      char buf[8];
      snprintf(buf, sizeof(buf), "\\u%04x", c);
      ss << buf;
      i++;
    } else if (c < 0x80) {
      ss << static_cast<char>(c);
      i++;
    } else {
      size_t seqLen = 0;
      if (c >= 0xC2 && c <= 0xDF) {
        seqLen = 2;
      } else if (c >= 0xE0 && c <= 0xEF) {
        seqLen = 3;
      } else if (c >= 0xF0 && c <= 0xF4) {
        seqLen = 4;
      }

      bool valid = (seqLen > 0 && i + seqLen <= str.size());
      if (valid) {
        for (size_t k = 1; k < seqLen; ++k) {
          if ((static_cast<unsigned char>(str[i + k]) & 0xC0) != 0x80) {
            valid = false;
            break;
          }
        }
        if (seqLen == 3) {
          unsigned char c1 = static_cast<unsigned char>(str[i + 1]);
          if (c == 0xE0 && c1 < 0xA0)
            valid = false;
          if (c == 0xED && c1 > 0x9F)
            valid = false;
        } else if (seqLen == 4) {
          unsigned char c1 = static_cast<unsigned char>(str[i + 1]);
          if (c == 0xF0 && c1 < 0x90)
            valid = false;
          if (c == 0xF4 && c1 > 0x8F)
            valid = false;
        }
      }

      if (valid) {
        for (size_t k = 0; k < seqLen; ++k) {
          ss << str[i + k];
        }
        i += seqLen;
      } else {
        char buf[8];
        snprintf(buf, sizeof(buf), "\\u%04x", c);
        ss << buf;
        i++;
      }
    }
  }
  return ss.str();
}

std::string ValueTypeToString(Value::Type t) {
  switch (t) {
  case Value::Type::Boolean:
    return "boolean";
  case Value::Type::Number:
    return "number";
  case Value::Type::Float:
    return "float";
  case Value::Type::String:
    return "string";
  case Value::Type::Array:
    return "array";
  default:
    return "unknown";
  }
}

// ---------------------------------------------------------------------------
// AST Traversal and Span Computation
// ---------------------------------------------------------------------------
void ComputeNodeSpan(SyntaxNode *node, int32_t &minStart, int32_t &maxEnd) {
  if (!node) {
    return;
  }
  if (auto *token = dynamic_cast<SyntaxToken *>(node)) {
    if (token->Kind() == SyntaxKind::EndOfFileToken) {
      return;
    }
    int32_t s = token->Span().Start();
    int32_t len = token->Span().Length();
    int32_t e = s + len;
    if (s >= 0 && e >= s) {
      if (minStart == -1 || s < minStart) {
        minStart = s;
      }
      if (maxEnd == -1 || e > maxEnd) {
        maxEnd = e;
      }
    }
    return;
  }
  for (SyntaxNode *child : node->GetChildren()) {
    ComputeNodeSpan(child, minStart, maxEnd);
  }
}

struct AstItem {
  std::string kind;
  int32_t start;
  int32_t end;
  std::string text;
  int depth;
  bool isToken;
};

void CollectAstNodes(SyntaxNode *node, const std::string &src, int depth,
                     std::vector<AstItem> &items) {
  if (!node) {
    return;
  }
  int32_t minStart = -1;
  int32_t maxEnd = -1;
  ComputeNodeSpan(node, minStart, maxEnd);

  if (dynamic_cast<CompilationUnitSyntaxNode *>(node)) {
    minStart = 0;
    maxEnd = static_cast<int32_t>(src.size());
  }

  std::string text;
  if (minStart >= 0 && maxEnd >= minStart &&
      static_cast<size_t>(maxEnd) <= src.size()) {
    text = src.substr(minStart, maxEnd - minStart);
  }

  std::string kindStr = "Unknown";
  auto it = SyntaxKindStrMap.find(node->Kind());
  if (it != SyntaxKindStrMap.end()) {
    kindStr = it->second;
  }
  bool isToken = (dynamic_cast<SyntaxToken *>(node) != nullptr);

  items.push_back({kindStr, minStart, maxEnd, text, depth, isToken});

  for (SyntaxNode *child : node->GetChildren()) {
    CollectAstNodes(child, src, depth + 1, items);
  }
}

// ---------------------------------------------------------------------------
// Symbol Extraction Data Structures
// ---------------------------------------------------------------------------
struct VariableSymbolInfo {
  std::string name;
  std::string type;
  std::string mutability;
  bool isReadOnly;
  int32_t start;
  int32_t end;
};

struct FunctionSymbolInfo {
  std::string name;
  std::string signature;
  std::string returnType;
  std::string doc;
  int32_t start;
  int32_t end;
  std::vector<std::pair<std::string, std::string>> params;
};

void CollectBoundSymbols(const BoundStatementNode *stmt,
                         std::map<std::string, VariableSymbolInfo> &vars,
                         std::map<std::string, FunctionSymbolInfo> &funcs) {
  if (!stmt) {
    return;
  }
  if (auto *block = dynamic_cast<const BoundBlockStatementNode *>(stmt)) {
    for (const auto &child : block->Statements()) {
      CollectBoundSymbols(child.get(), vars, funcs);
    }
  } else if (auto *ifStmt = dynamic_cast<const BoundIfStatementNode *>(stmt)) {
    CollectBoundSymbols(ifStmt->ThenStatement(), vars, funcs);
    if (ifStmt->ElseStatement()) {
      CollectBoundSymbols(ifStmt->ElseStatement(), vars, funcs);
    }
  } else if (auto *whileStmt =
                 dynamic_cast<const BoundWhileStatementNode *>(stmt)) {
    CollectBoundSymbols(whileStmt->Body(), vars, funcs);
  } else if (auto *doWhileStmt =
                 dynamic_cast<const BoundDoWhileStatementNode *>(stmt)) {
    CollectBoundSymbols(doWhileStmt->Body(), vars, funcs);
  } else if (auto *forStmt =
                 dynamic_cast<const BoundForStatementNode *>(stmt)) {
    if (forStmt->Initializer()) {
      CollectBoundSymbols(forStmt->Initializer(), vars, funcs);
    }
    CollectBoundSymbols(forStmt->Body(), vars, funcs);
  } else if (auto *varDecl =
                 dynamic_cast<const BoundVariableDeclarationNode *>(stmt)) {
    if (auto var = varDecl->Variable()) {
      std::string name = var->Name();
      bool isReadOnly = var->IsReadOnly();
      std::string type = ValueTypeToString(var->Type());
      if (var->Type() == Value::Type::Array &&
          var->ElementType() != Value::Type::Unknown) {
        type = "array<" + ValueTypeToString(var->ElementType()) + ">";
      }
      vars[name] = {name, type, isReadOnly ? "let" : "var", isReadOnly, -1, -1};
    }
  } else if (auto *fnDecl =
                 dynamic_cast<const BoundFunctionDeclarationNode *>(stmt)) {
    if (auto fn = fnDecl->Function()) {
      std::string name = fn->Name();
      std::string retType = ValueTypeToString(fn->ReturnType());
      std::string sig = "function " + name + "(";
      std::vector<std::pair<std::string, std::string>> params;
      for (size_t i = 0; i < fn->Parameters().size(); ++i) {
        if (i > 0) {
          sig += ", ";
        }
        const auto &p = fn->Parameters()[i];
        std::string pName = p ? p->Name() : "arg";
        std::string pType = p ? ValueTypeToString(p->Type()) : "any";
        sig += pName + ": " + pType;
        params.push_back({pName, pType});
      }
      sig += "): " + retType;
      funcs[name] = {name, sig, retType, "User-defined function " + name,
                     -1,   -1,  params};
      if (fn->Body()) {
        try {
          Binder fnBinder;
          auto boundBody = fnBinder.BindStatement(
              const_cast<BlockStatementSyntaxNode *>(fn->Body()));
          if (boundBody) {
            CollectBoundSymbols(boundBody.get(), vars, funcs);
          }
        } catch (...) {
        }
      }
    }
  }
}

void CollectSyntaxSymbols(SyntaxNode *node, const std::string &src,
                          std::map<std::string, VariableSymbolInfo> &vars,
                          std::map<std::string, FunctionSymbolInfo> &funcs) {
  if (!node) {
    return;
  }
  if (auto *varNode = dynamic_cast<VariableDeclarationSyntaxNode *>(node)) {
    if (auto idToken = varNode->Identifier()) {
      std::string name = idToken->Text();
      if (name.empty()) {
        int32_t s = idToken->Span().Start();
        int32_t len = idToken->Span().Length();
        if (s >= 0 && len > 0 && static_cast<size_t>(s + len) <= src.size()) {
          name = src.substr(s, len);
        }
      }
      if (!name.empty()) {
        int32_t s = -1;
        int32_t e = -1;
        ComputeNodeSpan(varNode, s, e);
        bool isLet = (varNode->Keyword() &&
                      varNode->Keyword()->Kind() == SyntaxKind::LetKeyword);
        std::string type = ValueTypeToString(varNode->TypeAnnotation());
        if (vars.find(name) != vars.end()) {
          if (s >= 0) {
            vars[name].start = s;
          }
          if (e >= 0) {
            vars[name].end = e;
          }
          if (vars[name].type == "unknown" && type != "unknown") {
            vars[name].type = type;
          }
        } else {
          vars[name] = {name, type, isLet ? "let" : "var", isLet, s, e};
        }
      }
    }
  } else if (auto *fnNode =
                 dynamic_cast<FunctionDeclarationSyntaxNode *>(node)) {
    if (auto idToken = fnNode->Identifier()) {
      std::string name = idToken->Text();
      if (name.empty()) {
        int32_t s = idToken->Span().Start();
        int32_t len = idToken->Span().Length();
        if (s >= 0 && len > 0 && static_cast<size_t>(s + len) <= src.size()) {
          name = src.substr(s, len);
        }
      }
      if (!name.empty()) {
        int32_t s = -1;
        int32_t e = -1;
        ComputeNodeSpan(fnNode, s, e);
        std::string retType = ValueTypeToString(fnNode->ReturnType());
        std::string sig =
            (fnNode->IsExtern() ? "extern function " : "function ") + name +
            "(";
        std::vector<std::pair<std::string, std::string>> params;
        for (size_t i = 0; i < fnNode->Parameters().size(); ++i) {
          if (i > 0) {
            sig += ", ";
          }
          std::string pName;
          if (fnNode->Parameters()[i]) {
            pName = fnNode->Parameters()[i]->Text();
            if (pName.empty()) {
              int32_t ps = fnNode->Parameters()[i]->Span().Start();
              int32_t plen = fnNode->Parameters()[i]->Span().Length();
              if (ps >= 0 && plen > 0 &&
                  static_cast<size_t>(ps + plen) <= src.size()) {
                pName = src.substr(ps, plen);
              }
            }
          }
          if (pName.empty()) {
            pName = "arg" + std::to_string(i);
          }
          std::string pType = "any";
          if (i < fnNode->ParameterTypes().size()) {
            pType = ValueTypeToString(fnNode->ParameterTypes()[i]);
          }
          sig += pName + ": " + pType;
          params.push_back({pName, pType});
        }
        sig += "): " + retType;
        if (funcs.find(name) != funcs.end()) {
          if (s >= 0) {
            funcs[name].start = s;
          }
          if (e >= 0) {
            funcs[name].end = e;
          }
          if (funcs[name].returnType == "unknown" && retType != "unknown") {
            funcs[name].returnType = retType;
            funcs[name].signature = sig;
          }
        } else {
          funcs[name] = {name, sig, retType, "Function " + name, s, e, params};
        }
      }
    }
  }

  for (SyntaxNode *child : node->GetChildren()) {
    CollectSyntaxSymbols(child, src, vars, funcs);
  }
}

// ---------------------------------------------------------------------------
// IR Formatting Helper
// ---------------------------------------------------------------------------
struct IrLineItem {
  std::string line;
  int32_t start;
  int32_t end;
};

std::vector<IrLineItem> FormatIrLines(const IrModule &module,
                                      const std::string &src) {
  std::vector<IrLineItem> lines;
  for (size_t i = 0; i < module.functions.size(); i++) {
    const auto &function = module.functions[i];
    if (function.isExtern) {
      std::ostringstream ss;
      ss << "extern func " << function.name << "(";
      for (size_t j = 0; j < function.parameters.size(); j++) {
        if (j != 0) {
          ss << ", ";
        }
        ss << function.parameters[j];
      }
      ss << ")";
      lines.push_back({ss.str(), -1, -1});
      if (i + 1 < module.functions.size()) {
        lines.push_back({"", -1, -1});
      }
      continue;
    }
    std::ostringstream fnHeader;
    fnHeader << "func " << function.name << "(";
    for (size_t j = 0; j < function.parameters.size(); j++) {
      if (j != 0) {
        fnHeader << ", ";
      }
      fnHeader << function.parameters[j];
    }
    fnHeader << ")";
    lines.push_back({fnHeader.str(), -1, -1});
    lines.push_back({IrLabelPrefix(IrLabelKind::Entry) + ":", -1, -1});

    for (const auto &instruction : function.instructions) {
      TextSpan span = GetInstructionSpan(instruction);
      int32_t s = span.Start();
      int32_t len = span.Length();
      int32_t e = (s >= 0 && len >= 0) ? (s + len) : -1;
      if (s < 0 || e < s || static_cast<size_t>(e) > src.size()) {
        s = -1;
        e = -1;
      }
      if (instruction.op == IrOpcode::Label) {
        lines.push_back({instruction.operands[0] + ":", s, e});
        continue;
      }
      std::ostringstream instLine;
      instLine << "  ";
      if (!instruction.result.empty()) {
        instLine << instruction.result << " = ";
      }
      instLine << IrOpcodeToString(instruction.op);
      for (const auto &operand : instruction.operands) {
        instLine << " " << operand;
      }
      lines.push_back({instLine.str(), s, e});
    }

    if (function.instructions.empty() ||
        function.instructions.back().op != IrOpcode::Ret) {
      lines.push_back({"  ret " + function.returnValue, -1, -1});
    }
    if (i + 1 < module.functions.size()) {
      lines.push_back({"", -1, -1});
    }
  }
  return lines;
}

} // namespace

// ---------------------------------------------------------------------------
// Global Persistent Return Buffers
// ---------------------------------------------------------------------------
char *g_ReturnBuffer = nullptr;
char *g_showSyntaxTree = nullptr;
char *g_showIR = nullptr;
char *g_AnalyzeBuffer = nullptr;
char *g_ExecuteBuffer = nullptr;

static void SetBuffer(char *&buffer, const std::string &value) {
  if (buffer) {
    delete[] buffer;
    buffer = nullptr;
  }
  buffer = new char[value.size() + 1];
  strcpy(buffer, value.c_str());
}

// ---------------------------------------------------------------------------
// C API Exports
// ---------------------------------------------------------------------------
extern "C" {

EMSCRIPTEN_KEEPALIVE
void InitWarf() { SymbolTableMgr::init(); }

EMSCRIPTEN_KEEPALIVE
char *ShowSyntaxTree() { return g_showSyntaxTree; }

EMSCRIPTEN_KEEPALIVE
char *ShowIR() { return g_showIR; }

EMSCRIPTEN_KEEPALIVE
char *RunWarf(char *input, bool showSyntaxTree, bool showIR) {
  std::string s(input ? input : "");

  SetBuffer(g_ReturnBuffer, "");
  SetBuffer(g_showSyntaxTree, "");
  SetBuffer(g_showIR, "");

  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(s);

  if (showSyntaxTree) {
    std::stringstream syntaxTreeStream;
    syntaxTree->PrintTree(syntaxTreeStream);
    SetBuffer(g_showSyntaxTree, syntaxTreeStream.str());
  }

  auto binder = std::make_unique<Binder>();
  std::unique_ptr<BoundStatementNode> boundStatement;
  std::stringstream outputStream;

  try {
    if (syntaxTree->Errors().empty()) {
      boundStatement = binder->BindCompilationUnit(syntaxTree->Root());
    }
  } catch (const std::exception &error) {
    outputStream << error.what() << std::endl;
  }

  if (!syntaxTree->Errors().empty() || !binder->Errors().empty()) {
    for (const auto &error : syntaxTree->Errors()) {
      outputStream << error << std::endl;
    }
    for (const auto &error : binder->Errors()) {
      outputStream << error << std::endl;
    }
    SetBuffer(g_ReturnBuffer, outputStream.str());
    return g_ReturnBuffer;
  }

  if (!boundStatement) {
    SetBuffer(g_ReturnBuffer, outputStream.str());
    return g_ReturnBuffer;
  }

  try {
    if (showIR) {
      IrLowerer lowerer;
      auto module = lowerer.Lower(boundStatement.get());

      std::stringstream irStream;
      IrPrinter::Print(module, irStream);
      SetBuffer(g_showIR, irStream.str());
    }

    auto eval = std::make_unique<Evaluator>(std::move(boundStatement));
    std::string capturedStdout;
    Value result;
    {
      CoutRedirect redirect;
      result = eval->Evaluate();
      capturedStdout = redirect.getString();
    }
    if (!capturedStdout.empty()) {
      outputStream << capturedStdout;
    }
    outputStream << result << std::endl;
  } catch (const std::exception &error) {
    outputStream << error.what() << std::endl;
  }

  SetBuffer(g_ReturnBuffer, outputStream.str());
  return g_ReturnBuffer;
}

EMSCRIPTEN_KEEPALIVE
const char *run_warf(const char *input) {
  return RunWarf(const_cast<char *>(input ? input : ""), false, false);
}

EMSCRIPTEN_KEEPALIVE
const char *emit_warf_ir(const char *input) {
  std::string s(input ? input : "");
  SetBuffer(g_showIR, "");

  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(s);
  auto binder = std::make_unique<Binder>();
  std::unique_ptr<BoundStatementNode> boundStatement;
  std::stringstream outStream;

  try {
    if (syntaxTree->Errors().empty()) {
      boundStatement = binder->BindCompilationUnit(syntaxTree->Root());
    }
  } catch (const std::exception &error) {
    outStream << error.what() << std::endl;
  }

  if (!syntaxTree->Errors().empty() || !binder->Errors().empty()) {
    for (const auto &error : syntaxTree->Errors()) {
      outStream << error << std::endl;
    }
    for (const auto &error : binder->Errors()) {
      outStream << error << std::endl;
    }
    SetBuffer(g_showIR, outStream.str());
    return g_showIR;
  }

  if (!boundStatement) {
    SetBuffer(g_showIR, outStream.str());
    return g_showIR;
  }

  try {
    IrLowerer lowerer;
    auto module = lowerer.Lower(boundStatement.get());
    IrPrinter::Print(module, outStream);
  } catch (const std::exception &error) {
    outStream << error.what() << std::endl;
  }

  SetBuffer(g_showIR, outStream.str());
  return g_showIR;
}

EMSCRIPTEN_KEEPALIVE
const char *ExecuteWarf(const char *source, int engine, int stepLimit) {
  std::string src(source ? source : "");
  auto start = std::chrono::high_resolution_clock::now();

  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(src);
  auto binder = std::make_unique<Binder>();
  std::unique_ptr<BoundStatementNode> boundStatement;
  std::stringstream errorStream;

  try {
    if (syntaxTree->Errors().empty()) {
      boundStatement = binder->BindCompilationUnit(syntaxTree->Root());
    }
  } catch (const std::exception &error) {
    errorStream << error.what() << "\n";
  }

  if (!syntaxTree->Errors().empty() || !binder->Errors().empty()) {
    for (const auto &error : syntaxTree->Errors()) {
      errorStream << error << "\n";
    }
    for (const auto &error : binder->Errors()) {
      errorStream << error << "\n";
    }
    auto end = std::chrono::high_resolution_clock::now();
    double elapsedMs =
        std::chrono::duration<double, std::milli>(end - start).count();

    std::ostringstream json;
    json << "{\n"
         << "  \"status\": \"error\",\n"
         << "  \"success\": false,\n"
         << "  \"stdout\": \"\",\n"
         << "  \"result\": \"\",\n"
         << "  \"resultType\": \"unknown\",\n"
         << "  \"error\": \"" << EscapeJsonString(errorStream.str()) << "\",\n"
         << "  \"elapsedMs\": " << elapsedMs << ",\n"
         << "  \"engine\": " << engine << ",\n"
         << "  \"stepLimit\": " << stepLimit << "\n"
         << "}";
    SetBuffer(g_ExecuteBuffer, json.str());
    return g_ExecuteBuffer;
  }

  if (!boundStatement) {
    auto end = std::chrono::high_resolution_clock::now();
    double elapsedMs =
        std::chrono::duration<double, std::milli>(end - start).count();

    std::ostringstream json;
    json << "{\n"
         << "  \"status\": \"error\",\n"
         << "  \"success\": false,\n"
         << "  \"stdout\": \"\",\n"
         << "  \"result\": \"\",\n"
         << "  \"resultType\": \"unknown\",\n"
         << "  \"error\": \""
         << EscapeJsonString(errorStream.str().empty()
                                 ? "No statement to execute"
                                 : errorStream.str())
         << "\",\n"
         << "  \"elapsedMs\": " << elapsedMs << ",\n"
         << "  \"engine\": " << engine << ",\n"
         << "  \"stepLimit\": " << stepLimit << "\n"
         << "}";
    SetBuffer(g_ExecuteBuffer, json.str());
    return g_ExecuteBuffer;
  }

  std::string capturedStdout;
  std::string resultStr;
  std::string resultTypeStr = "unknown";
  std::string errStr;
  bool success = true;

  try {
    CoutRedirect redirect;
    Value result;
    if (engine == 1 || engine == 2) {
      IrLowerer lowerer;
      auto module = lowerer.Lower(boundStatement.get());
      if (engine == 2) {
        module = IrOptimizer::Optimize(module);
      }
      IrInterpreter interpreter;
      ApplyStepLimit(interpreter, stepLimit);
      result = interpreter.Execute(module);
    } else {
      auto eval = std::make_unique<Evaluator>(std::move(boundStatement));
      ApplyStepLimit(*eval, stepLimit);
      result = eval->Evaluate();
    }
    capturedStdout = redirect.getString();

    std::ostringstream resStream;
    resStream << result;
    resultStr = resStream.str();
    resultTypeStr = ValueTypeToString(result.VType());
  } catch (const std::exception &error) {
    success = false;
    errStr = error.what();
  }

  auto end = std::chrono::high_resolution_clock::now();
  double elapsedMs =
      std::chrono::duration<double, std::milli>(end - start).count();

  std::ostringstream json;
  json << "{\n"
       << "  \"status\": \"" << (success ? "ok" : "error") << "\",\n"
       << "  \"success\": " << (success ? "true" : "false") << ",\n"
       << "  \"stdout\": \"" << EscapeJsonString(capturedStdout) << "\",\n"
       << "  \"result\": \"" << EscapeJsonString(resultStr) << "\",\n"
       << "  \"resultType\": \"" << EscapeJsonString(resultTypeStr) << "\",\n"
       << "  \"error\": \"" << EscapeJsonString(errStr) << "\",\n"
       << "  \"elapsedMs\": " << elapsedMs << ",\n"
       << "  \"engine\": " << engine << ",\n"
       << "  \"stepLimit\": " << stepLimit << "\n"
       << "}";

  SetBuffer(g_ExecuteBuffer, json.str());
  return g_ExecuteBuffer;
}

EMSCRIPTEN_KEEPALIVE
const char *AnalyzeWarf(const char *source) {
  std::string src(source ? source : "");

  // 1. Token Stream
  std::ostringstream tokensJson;
  tokensJson << "[\n";
  {
    Lexer lexer(src);
    bool first = true;
    while (true) {
      auto token = lexer.NextToken();
      if (!token) {
        break;
      }
      int32_t startPos = token->Span().Start();
      int32_t length = token->Span().Length();
      int32_t endPos = (token->Kind() == SyntaxKind::EndOfFileToken)
                           ? startPos
                           : (startPos + length);
      std::string kindStr = "UnknownToken";
      auto it = SyntaxKindStrMap.find(token->Kind());
      if (it != SyntaxKindStrMap.end()) {
        kindStr = it->second;
      }
      std::string tokenText = token->Text();
      if (tokenText.empty() && startPos >= 0 && length > 0 &&
          static_cast<size_t>(startPos + length) <= src.size()) {
        tokenText = src.substr(startPos, length);
      }

      LineColumn startLC =
          TextSpan::ComputeLineColumn(src, startPos >= 0 ? startPos : 0);
      LineColumn endLC =
          TextSpan::ComputeLineColumn(src, endPos >= 0 ? endPos : 0);

      if (!first) {
        tokensJson << ",\n";
      }
      first = false;

      tokensJson << "    {"
                 << "\"start\": " << startPos << ", "
                 << "\"end\": " << endPos << ", "
                 << "\"kind\": \"" << EscapeJsonString(kindStr) << "\", "
                 << "\"text\": \"" << EscapeJsonString(tokenText) << "\", "
                 << "\"startLine\": " << startLC.line << ", "
                 << "\"startCol\": " << startLC.column << ", "
                 << "\"endLine\": " << endLC.line << ", "
                 << "\"endCol\": " << endLC.column << "}";

      if (token->Kind() == SyntaxKind::EndOfFileToken) {
        break;
      }
    }
  }
  tokensJson << "\n  ]";

  // 2. Syntax and Binder Diagnostics
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(src);
  auto binder = std::make_unique<Binder>();
  std::unique_ptr<BoundStatementNode> boundStatement;

  try {
    if (syntaxTree->Errors().empty()) {
      boundStatement = binder->BindCompilationUnit(syntaxTree->Root());
    }
  } catch (...) {
  }

  std::ostringstream diagJson;
  diagJson << "[\n";
  bool firstDiag = true;
  for (const auto &err : syntaxTree->Errors()) {
    int32_t s = err.Span().Start();
    int32_t len = err.Span().Length();
    int32_t e = (s >= 0 && len >= 0) ? (s + len) : -1;
    LineColumn startLC = TextSpan::ComputeLineColumn(src, s >= 0 ? s : 0);
    LineColumn endLC =
        TextSpan::ComputeLineColumn(src, e >= 0 ? e : (s >= 0 ? s : 0));
    if (!firstDiag) {
      diagJson << ",\n";
    }
    firstDiag = false;
    diagJson << "    {"
             << "\"start\": " << s << ", "
             << "\"end\": " << e << ", "
             << "\"message\": \"" << EscapeJsonString(err.Message()) << "\", "
             << "\"severity\": \"error\", "
             << "\"stage\": \"syntax\", "
             << "\"startLine\": " << startLC.line << ", "
             << "\"startCol\": " << startLC.column << ", "
             << "\"endLine\": " << endLC.line << ", "
             << "\"endCol\": " << endLC.column << "}";
  }
  for (const auto &err : binder->Errors()) {
    int32_t s = err.Span().Start();
    int32_t len = err.Span().Length();
    int32_t e = (s >= 0 && len >= 0) ? (s + len) : -1;
    LineColumn startLC = TextSpan::ComputeLineColumn(src, s >= 0 ? s : 0);
    LineColumn endLC =
        TextSpan::ComputeLineColumn(src, e >= 0 ? e : (s >= 0 ? s : 0));
    if (!firstDiag) {
      diagJson << ",\n";
    }
    firstDiag = false;
    diagJson << "    {"
             << "\"start\": " << s << ", "
             << "\"end\": " << e << ", "
             << "\"message\": \"" << EscapeJsonString(err.Message()) << "\", "
             << "\"severity\": \"error\", "
             << "\"stage\": \"binder\", "
             << "\"startLine\": " << startLC.line << ", "
             << "\"startCol\": " << startLC.column << ", "
             << "\"endLine\": " << endLC.line << ", "
             << "\"endCol\": " << endLC.column << "}";
  }
  diagJson << "\n  ]";

  bool hasErrors = !syntaxTree->Errors().empty() || !binder->Errors().empty();

  // 3. AST Nodes
  std::vector<AstItem> astItems;
  if (syntaxTree->Root()) {
    CollectAstNodes(syntaxTree->Root(), src, 0, astItems);
  }

  std::ostringstream astJson;
  astJson << "[\n";
  for (size_t i = 0; i < astItems.size(); ++i) {
    if (i > 0) {
      astJson << ",\n";
    }
    astJson << "    {"
            << "\"kind\": \"" << EscapeJsonString(astItems[i].kind) << "\", "
            << "\"start\": " << astItems[i].start << ", "
            << "\"end\": " << astItems[i].end << ", "
            << "\"text\": \"" << EscapeJsonString(astItems[i].text) << "\", "
            << "\"depth\": " << astItems[i].depth << ", "
            << "\"isToken\": " << (astItems[i].isToken ? "true" : "false")
            << "}";
  }
  astJson << "\n  ]";

  std::stringstream stStream;
  syntaxTree->PrintTree(stStream);
  std::string syntaxTreeText = stStream.str();

  // 4. WarfIR Generation (O0 and Optimized)
  std::vector<IrLineItem> irO0Lines;
  std::vector<IrLineItem> irOLines;
  std::string irO0Text;
  std::string irOText;

  if (boundStatement) {
    try {
      IrLowerer lowerer;
      auto moduleO0 = lowerer.Lower(boundStatement.get());
      auto moduleO = IrOptimizer::Optimize(moduleO0);

      irO0Lines = FormatIrLines(moduleO0, src);
      irOLines = FormatIrLines(moduleO, src);

      std::stringstream ss0;
      IrPrinter::Print(moduleO0, ss0);
      irO0Text = ss0.str();

      std::stringstream ssOpt;
      IrPrinter::Print(moduleO, ssOpt);
      irOText = ssOpt.str();
    } catch (...) {
    }
  }

  auto FormatLinesJson = [](const std::vector<IrLineItem> &lines) {
    std::ostringstream ss;
    ss << "[\n";
    for (size_t i = 0; i < lines.size(); ++i) {
      if (i > 0) {
        ss << ",\n";
      }
      ss << "    {"
         << "\"line\": \"" << EscapeJsonString(lines[i].line) << "\", "
         << "\"text\": \"" << EscapeJsonString(lines[i].line) << "\", "
         << "\"start\": " << lines[i].start << ", "
         << "\"end\": " << lines[i].end << "}";
    }
    ss << "\n  ]";
    return ss.str();
  };

  // 5. Symbol Tables (Keywords, Builtins, Variables, Functions)
  std::map<std::string, VariableSymbolInfo> varSymbols;
  std::map<std::string, FunctionSymbolInfo> fnSymbols;

  if (boundStatement) {
    CollectBoundSymbols(boundStatement.get(), varSymbols, fnSymbols);
  }
  if (syntaxTree->Root()) {
    CollectSyntaxSymbols(syntaxTree->Root(), src, varSymbols, fnSymbols);
  }

  std::ostringstream symbolsJson;
  symbolsJson << "{\n";

  // Keywords
  symbolsJson << "    \"keywords\": [\n"
              << "      {\"name\": \"break\", \"kind\": \"keyword\", "
                 "\"detail\": \"break statement\"},\n"
              << "      {\"name\": \"continue\", \"kind\": \"keyword\", "
                 "\"detail\": \"continue statement\"},\n"
              << "      {\"name\": \"if\", \"kind\": \"keyword\", \"detail\": "
                 "\"if condition statement\"},\n"
              << "      {\"name\": \"else\", \"kind\": \"keyword\", "
                 "\"detail\": \"else conditional branch\"},\n"
              << "      {\"name\": \"true\", \"kind\": \"keyword\", "
                 "\"detail\": \"boolean literal true\"},\n"
              << "      {\"name\": \"false\", \"kind\": \"keyword\", "
                 "\"detail\": \"boolean literal false\"},\n"
              << "      {\"name\": \"for\", \"kind\": \"keyword\", \"detail\": "
                 "\"for loop statement\"},\n"
              << "      {\"name\": \"while\", \"kind\": \"keyword\", "
                 "\"detail\": \"while loop statement\"},\n"
              << "      {\"name\": \"do\", \"kind\": \"keyword\", \"detail\": "
                 "\"do-while loop statement\"},\n"
              << "      {\"name\": \"function\", \"kind\": \"keyword\", "
                 "\"detail\": \"function declaration\"},\n"
              << "      {\"name\": \"extern\", \"kind\": \"keyword\", "
                 "\"detail\": \"extern function declaration\"},\n"
              << "      {\"name\": \"return\", \"kind\": \"keyword\", "
                 "\"detail\": \"return statement\"},\n"
              << "      {\"name\": \"let\", \"kind\": \"keyword\", \"detail\": "
                 "\"immutable variable declaration\"},\n"
              << "      {\"name\": \"var\", \"kind\": \"keyword\", \"detail\": "
                 "\"mutable variable declaration\"}\n"
              << "    ],\n";

  // Built-ins
  symbolsJson
      << "    \"builtins\": [\n"
      << "      {\n"
      << "        \"name\": \"print\",\n"
      << "        \"kind\": \"function\",\n"
      << "        \"signature\": \"function print(val: any): any\",\n"
      << "        \"returnType\": \"any\",\n"
      << "        \"doc\": \"Prints a value to stdout\",\n"
      << "        \"params\": [{\"name\": \"val\", \"type\": \"any\"}]\n"
      << "      },\n"
      << "      {\n"
      << "        \"name\": \"len\",\n"
      << "        \"kind\": \"function\",\n"
      << "        \"signature\": \"function len(arr: array): number\",\n"
      << "        \"returnType\": \"number\",\n"
      << "        \"doc\": \"Returns the length of an array\",\n"
      << "        \"params\": [{\"name\": \"arr\", \"type\": \"array\"}]\n"
      << "      }\n"
      << "    ],\n";

  // Variables
  symbolsJson << "    \"variables\": [\n";
  bool firstVar = true;
  for (const auto &pair : varSymbols) {
    if (!firstVar) {
      symbolsJson << ",\n";
    }
    firstVar = false;
    const auto &v = pair.second;
    symbolsJson << "      {"
                << "\"name\": \"" << EscapeJsonString(v.name) << "\", "
                << "\"kind\": \"variable\", "
                << "\"type\": \"" << EscapeJsonString(v.type) << "\", "
                << "\"mutability\": \"" << EscapeJsonString(v.mutability)
                << "\", "
                << "\"isReadOnly\": " << (v.isReadOnly ? "true" : "false")
                << ", "
                << "\"start\": " << v.start << ", "
                << "\"end\": " << v.end << "}";
  }
  symbolsJson << "\n    ],\n";

  // Functions
  symbolsJson << "    \"functions\": [\n";
  bool firstFn = true;
  for (const auto &pair : fnSymbols) {
    if (!firstFn) {
      symbolsJson << ",\n";
    }
    firstFn = false;
    const auto &f = pair.second;
    symbolsJson << "      {\n"
                << "        \"name\": \"" << EscapeJsonString(f.name) << "\",\n"
                << "        \"kind\": \"function\",\n"
                << "        \"signature\": \"" << EscapeJsonString(f.signature)
                << "\",\n"
                << "        \"returnType\": \""
                << EscapeJsonString(f.returnType) << "\",\n"
                << "        \"doc\": \"" << EscapeJsonString(f.doc) << "\",\n"
                << "        \"start\": " << f.start << ",\n"
                << "        \"end\": " << f.end << ",\n"
                << "        \"params\": [";
    for (size_t pIdx = 0; pIdx < f.params.size(); ++pIdx) {
      if (pIdx > 0) {
        symbolsJson << ", ";
      }
      symbolsJson << "{\"name\": \"" << EscapeJsonString(f.params[pIdx].first)
                  << "\", \"type\": \""
                  << EscapeJsonString(f.params[pIdx].second) << "\"}";
    }
    symbolsJson << "]\n      }";
  }
  symbolsJson << "\n    ]\n";
  symbolsJson << "  }";

  // Final Output JSON
  std::ostringstream json;
  json << "{\n"
       << "  \"source\": \"" << EscapeJsonString(src) << "\",\n"
       << "  \"hasErrors\": " << (hasErrors ? "true" : "false") << ",\n"
       << "  \"tokens\": " << tokensJson.str() << ",\n"
       << "  \"diagnostics\": " << diagJson.str() << ",\n"
       << "  \"ast\": " << astJson.str() << ",\n"
       << "  \"syntaxTreeText\": \"" << EscapeJsonString(syntaxTreeText)
       << "\",\n"
       << "  \"irO0\": " << FormatLinesJson(irO0Lines) << ",\n"
       << "  \"irO\": " << FormatLinesJson(irOLines) << ",\n"
       << "  \"irOpt\": " << FormatLinesJson(irOLines) << ",\n"
       << "  \"irO0Text\": \"" << EscapeJsonString(irO0Text) << "\",\n"
       << "  \"irOText\": \"" << EscapeJsonString(irOText) << "\",\n"
       << "  \"symbols\": " << symbolsJson.str() << "\n"
       << "}";

  SetBuffer(g_AnalyzeBuffer, json.str());
  return g_AnalyzeBuffer;
}

} // extern "C"
