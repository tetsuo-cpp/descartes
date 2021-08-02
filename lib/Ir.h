#pragma once

#include "SymbolTable.h"

namespace descartes::ir {

// TODO: Abstract out ARM specific details.
static int wordSize = 8;

struct Access {
  explicit Access(int offset) : offset(offset) {}
  int offset;
};

struct Level {
  explicit Level(Symbol name) : name(name) {}
  void allocLocal() {
    int offset = locals.size() * wordSize;
    locals.emplace_back(-offset);
  }
  const Symbol name;
  std::vector<Access> locals;
};

using StatementPtr = std::unique_ptr<Statement>;
struct Statement {
  virtual ~Statement() = default;
};

using ExprPtr = std::unique_ptr<Expr>;
struct Expr {
  virtual ~Expr() = default;
};

struct Sequence : public Statement {
  explicit Sequence(std::vector<StatementPtr> &&statements)
      : statements(std::move(statements)) {}
  const std::vector<StatementPtr> statements;
};

struct Label : public Statement {
  explicit Label(Symbol label) : label(label) {}
  const Symbol label;
};

enum class RelOp {
  Equal,
  NotEqual,
  LessThan,
  GreaterThan,
  LessThanEqual,
  GreaterThanEqual,
  LessThanUnsigned,
  GreaterThanUnsigned,
  LessThanEqualUnsigned,
  GreaterThanEqualUnsigned,
};

struct Jump : public Statement {
  Jump(Symbol jumpLabel) : jumpLabel(jumpLabel) {}
  const Symbol jumpLabel;
};

struct CondJump : public Statement {
  CondJump(RelOp op, ExprPtr lhs, ExprPtr rhs, Symbol trueLabel,
           Symbol elseLabel)
      : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)), trueLabel(trueLabel),
        elseLabel(elseLabel) {}
  const RelOp op;
  const ExprPtr lhs, rhs;
  const Symbol trueLabel, elseLabel;
};

struct Move : public Statement {
  Move(ExprPtr dst, ExprPtr src) : dst(std::move(dst)), src(std::move(src)) {}
  const ExprPtr dst, src;
};

struct CallStatement : public Statement {
  explicit CallStatement(ExprPtr call) : call(std::move(call)) {}
  const ExprPtr call;
};

struct BinOp : public Expr {
  // BinOp op;
  const ExprPtr lhs, rhs;
};

struct Mem : public Expr {
  explicit Mem(ExprPtr expr) : expr(std::move(expr)) {}
  const ExprPtr expr;
};

struct Name : public Expr {
  explicit Name(Symbol name) : name(name) {}
  Symbol name;
};

struct Const : public Expr {
  explicit Const(int value) : value(value) {}
  int value;
};

struct Call : public Expr {
  Call(Symbol functionName, std::vector<ExprPtr> args)
      : functionName(functionName), args(std::move(args)) {}
  Symbol functionName;
  std::vector<ExprPtr> args;
};

} // namespace descartes::ir
