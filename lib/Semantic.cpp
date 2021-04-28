#include <Semantic.h>

#include <cassert>

namespace descartes {

Semantic::Semantic(const SymbolTable &symbols)
    : symbols(symbols), typeDefs(nullptr) {
  static_cast<void>(this->symbols);
}

void Semantic::analyse(Block &program) { analyseBlock(program); }

void Semantic::analyseBlock(Block &program) {
  analyseConstDefs(program.constDefs);
  analyseTypeDefs(program.typeDefs);
  analyseVarDecls(program.varDecls);
  analyseFunctions(program.functions);
  analyseStatements(*program.statements);
}

void Semantic::analyseConstDefs(const std::vector<ConstDef> &constDefs) {
  static_cast<void>(constDefs);
}

void Semantic::analyseTypeDefs(const TypeDefs &typeDefs) {
  this->typeDefs = &typeDefs;
}

void Semantic::analyseVarDecls(const VarDecls &varDecls) {
  this->varDecls = &varDecls;
}

void Semantic::analyseFunctions(
    const std::vector<std::unique_ptr<Function>> &functions) {
  this->functions = &functions;
}

void Semantic::analyseStatements(Statement &statement) {
  auto *compound = statementCast<Compound *>(statement);
  if (!compound)
    throw SemanticError("Block body must be a compound statement");
  for (const auto &s : compound->body)
    analyseStatement(*s);
}

void Semantic::analyseStatement(Statement &statement) {
  switch (statement.getKind()) {
  case StatementKind::Assignment:
    analyseAssignment(statement);
    break;
  case StatementKind::Compound:
    analyseCompound(statement);
    break;
  case StatementKind::If:
    analyseIf(statement);
    break;
  case StatementKind::Case:
    analyseCase(statement);
    break;
  case StatementKind::While:
    analyseWhile(statement);
    break;
  case StatementKind::Call:
    analyseCallStatement(statement);
    break;
  default:
    assert(!"Unsupported statement kind");
  }
}

void Semantic::analyseAssignment(Statement &statement) {
  // TODO: Handle const.
  auto *assignment = statementCast<Assignment *>(statement);
  assert(assignment);
  const Type *lhs = analyseExpr(*assignment->lhs),
             *rhs = analyseExpr(*assignment->rhs);
  if (!isCompatibleType(lhs, rhs))
    throw SemanticError("Assignment error");
}

void Semantic::analyseCompound(Statement &statement) {
  auto *compound = statementCast<Compound *>(statement);
  assert(compound);
  for (const auto &s : compound->body)
    analyseStatement(*s);
}

void Semantic::analyseIf(Statement &statement) {
  auto *ifStatement = statementCast<If *>(statement);
  assert(ifStatement);
  const auto *condType = analyseExpr(*ifStatement->cond);
  if (resolveType(condType)->getKind() != TypeKind::Boolean)
    throw SemanticError("If condition must be boolean");
  analyseStatement(*ifStatement->thenStatement);
  if (ifStatement->elseStatement)
    analyseStatement(*ifStatement->elseStatement);
}

void Semantic::analyseCase(Statement &statement) {
  static_cast<void>(statement);
}

void Semantic::analyseWhile(Statement &statement) {
  auto *whileStatement = statementCast<While *>(statement);
  assert(whileStatement);
  const auto *condType = analyseExpr(*whileStatement->cond);
  if (resolveType(condType)->getKind() != TypeKind::Boolean)
    throw SemanticError("While condition must be a boolean");
  analyseStatement(*whileStatement->body);
}

void Semantic::analyseCallStatement(Statement &statement) {
  auto *callStatement = statementCast<CallStatement *>(statement);
  assert(callStatement);
  auto *call = exprCast<Call *>(*callStatement->call);
  if (!call)
    throw SemanticError("Call statement with a non-call node within");
  analyseExpr(*call);
}

const Type *Semantic::analyseExpr(Expr &expr) {
  static_cast<void>(expr);
  return nullptr;
}

const Type *Semantic::resolveType(const Type *type) const {
  const Type *resolvedType = type;
  while (resolvedType->getKind() == TypeKind::Alias) {
    const Symbol typeIdentifier =
        static_cast<const Alias *>(resolvedType)->typeIdentifier;
    const auto iter = typeDefs->find(typeIdentifier);
    if (iter == typeDefs->end())
      throw SemanticError("Could not resolve type");
    resolvedType = iter->second.get();
  }
  return resolvedType;
}

bool Semantic::isCompatibleType(const Type *lhs, const Type *rhs) const {
  lhs = resolveType(lhs);
  rhs = resolveType(rhs);
  // Different resolved kinds are always incompatible.
  if (lhs->getKind() != rhs->getKind())
    return false;
  switch (lhs->getKind()) {
  case TypeKind::Integer:
  case TypeKind::Boolean:
    return true;
  case TypeKind::Record:
  case TypeKind::Enum:
    return lhs == rhs;
  case TypeKind::Alias:
    break;
  }
  throw SemanticError("Unreachable");
}

} // namespace descartes
