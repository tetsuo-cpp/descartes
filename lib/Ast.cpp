#include "Ast.h"

namespace descartes {

Block::Block(std::vector<std::string> &&labelDecl, std::vector<AstPtr> constDef,
             AstPtr typeDef, AstPtr varDecl, AstPtr functions,
             AstPtr statements)
    : labelDecl(std::move(labelDecl)), constDef(std::move(constDef)),
      typeDef(std::move(typeDef)), varDecl(std::move(varDecl)),
      functions(std::move(functions)), statements(std::move(statements)) {}

AstKind Block::getKind() const { return AstKind::Block; }

ConstDef::ConstDef(std::string &&identifier, AstPtr constExpr)
    : identifier(std::move(identifier)), constExpr(std::move(constExpr)) {}

AstKind ConstDef::getKind() const { return AstKind::ConstDef; }

StringLiteral::StringLiteral(const std::string &val) : val(val) {}

AstKind StringLiteral::getKind() const { return AstKind::StringLiteral; }

NumberLiteral::NumberLiteral(int val) : val(val) {}

AstKind NumberLiteral::getKind() const { return AstKind::NumberLiteral; }

VariableRef::VariableRef(const std::string &identifier)
    : identifier(identifier) {}

AstKind VariableRef::getKind() const { return AstKind::VariableRef; }

} // namespace descartes
