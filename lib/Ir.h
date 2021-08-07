#pragma once

#include "SymbolTable.h"

namespace descartes::ir {

// TODO: Abstract out ARM specific details.
static int wordSize = 8;

// TODO: Implement escape detection and use registers for non-escaping args.
struct Level;
struct Access {
  explicit Access(Level *level, int offset) : level(level), offset(offset) {}
  Level *level;
  int offset;
};

struct Level {
  explicit Level(Symbol name) : name(name) {}
  Access allocLocal() {
    int offset = locals.size() * wordSize;
    locals.emplace_back(this, -offset);
    return locals.back();
  }
  const Symbol name;
  std::vector<Access> locals;
};

using Fragment = std::pair<Level, StatementPtr>;

enum class StatementKind {
  Sequence,
  Label,
  Jump,
  CondJump,
  Move,
  CallStatement,
};
struct Statement {
  virtual ~Statement() = default;
  virtual StatementKind getKind() const = 0;
};
using StatementPtr = std::unique_ptr<Statement>;

enum class ExprKind {
  ArithOp,
  Mem,
  Name,
  Const,
  Call,
  CondExpr,
};
struct Expr {
  virtual ~Expr() = default;
  virtual ExprKind getKind() const = 0;
};
using ExprPtr = std::unique_ptr<Expr>;

struct Sequence : public Statement {
  explicit Sequence(std::vector<StatementPtr> &&statements)
      : statements(std::move(statements)) {}
  StatementKind getKind() const override { return StatementKind::Sequence; }
  const std::vector<StatementPtr> statements;
};

struct Label : public Statement {
  explicit Label(Symbol label) : label(label) {}
  StatementKind getKind() const override { return StatementKind::Label; }
  const Symbol label;
};

enum class RelOpKind {
  Equal,
  NotEqual,
  LessThan,
  GreaterThan,
  LessThanEqual,
  GreaterThanEqual,
  // TODO: Figure out how to deal with unsigned comparisons
  // LessThanUnsigned,
  // GreaterThanUnsigned,
  // LessThanEqualUnsigned,
  // GreaterThanEqualUnsigned,
};

struct Jump : public Statement {
  Jump(Symbol jumpLabel) : jumpLabel(jumpLabel) {}
  StatementKind getKind() const override { return StatementKind::Jump; }
  const Symbol jumpLabel;
};

struct CondJump : public Statement {
  CondJump(RelOpKind op, ExprPtr lhs, ExprPtr rhs, Symbol thenLabel,
           Symbol elseLabel)
      : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)), thenLabel(thenLabel),
        elseLabel(elseLabel) {}
  StatementKind getKind() const override { return StatementKind::CondJump; }
  const RelOpKind op;
  const ExprPtr lhs, rhs;
  const Symbol thenLabel, elseLabel;
};

struct Move : public Statement {
  Move(ExprPtr dst, ExprPtr src) : dst(std::move(dst)), src(std::move(src)) {}
  StatementKind getKind() const override { return StatementKind::Move; }
  const ExprPtr dst, src;
};

struct CallStatement : public Statement {
  explicit CallStatement(ExprPtr call) : call(std::move(call)) {}
  StatementKind getKind() const override {
    return StatementKind::CallStatement;
  }
  const ExprPtr call;
};

enum class ArithOpKind {
  Add,
  Subtract,
  Multiply,
  Divide,
};

struct ArithOp : public Expr {
  ArithOp(ArithOpKind op, ExprPtr lhs, ExprPtr rhs)
      : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  ExprKind getKind() const override { return ExprKind::ArithOp; }
  ArithOpKind op;
  const ExprPtr lhs, rhs;
};

struct Mem : public Expr {
  explicit Mem(ExprPtr expr) : expr(std::move(expr)) {}
  ExprKind getKind() const override { return ExprKind::Mem; }
  const ExprPtr expr;
};

struct Name : public Expr {
  explicit Name(Symbol value) : value(value) {}
  ExprKind getKind() const override { return ExprKind::Name; }
  Symbol value;
};

struct Const : public Expr {
  explicit Const(int value) : value(value) {}
  ExprKind getKind() const override { return ExprKind::Const; }
  int value;
};

struct Call : public Expr {
  Call(Symbol functionName, std::vector<ExprPtr> args)
      : functionName(functionName), args(std::move(args)) {}
  ExprKind getKind() const override { return ExprKind::Call; }
  Symbol functionName;
  std::vector<ExprPtr> args;
};

struct CondExpr : public Expr {
  explicit CondExpr(StatementPtr condJump) : condJump(std::move(condJump)) {}
  ExprKind getKind() const override { return ExprKind::CondExpr; }
  StatementPtr condJump;
};

} // namespace descartes::ir
