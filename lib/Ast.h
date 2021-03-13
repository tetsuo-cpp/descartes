#pragma once

#include <Interfaces.h>

#include <memory>
#include <vector>

namespace descartes {

enum class AstKind {
  Block,
  ConstDef,
  StringLiteral,
  NumberLiteral,
  VariableRef,
};

struct Block : public IAst {
  Block(std::vector<std::string> &&labelDecl, std::vector<AstPtr> constDef,
        AstPtr typeDef, AstPtr varDecl, AstPtr functions, AstPtr statements);
  AstKind getKind() const override;
  std::vector<std::string> labelDecl;
  std::vector<AstPtr> constDef;
  AstPtr typeDef, varDecl, functions, statements;
};

struct ConstDef : public IAst {
  ConstDef(std::string &&identifier, AstPtr constExpr);
  AstKind getKind() const override;
  std::string identifier;
  AstPtr constExpr;
};

struct StringLiteral : public IAst {
  explicit StringLiteral(const std::string &val);
  AstKind getKind() const override;
  std::string val;
};

struct NumberLiteral : public IAst {
  explicit NumberLiteral(int val);
  AstKind getKind() const override;
  int val;
};

struct VariableRef : public IAst {
  explicit VariableRef(const std::string &identifier);
  AstKind getKind() const override;
  std::string identifier;
};

template <typename T, AstKind kind> inline T astCastImpl(IAst &ast) {
  if (ast.getKind() == kind)
    return static_cast<T>(&ast);
  return nullptr;
}

template <typename T> inline T astCast(IAst &) {
  static_assert(sizeof(T) != sizeof(T),
                "astCast must be specialised for this type");
}

template <> inline Block *astCast<Block *>(IAst &ast) {
  return astCastImpl<Block *, AstKind::Block>(ast);
}

template <> inline ConstDef *astCast<ConstDef *>(IAst &ast) {
  return astCastImpl<ConstDef *, AstKind::ConstDef>(ast);
}

template <> inline StringLiteral *astCast<StringLiteral *>(IAst &ast) {
  return astCastImpl<StringLiteral *, AstKind::StringLiteral>(ast);
}

template <> inline NumberLiteral *astCast<NumberLiteral *>(IAst &ast) {
  return astCastImpl<NumberLiteral *, AstKind::NumberLiteral>(ast);
}

template <> inline VariableRef *astCast<VariableRef *>(IAst &ast) {
  return astCastImpl<VariableRef *, AstKind::VariableRef>(ast);
}

} // namespace descartes
