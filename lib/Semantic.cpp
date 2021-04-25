#include <Semantic.h>

namespace descartes {

Semantic::Semantic(const SymbolTable &symbols) : symbols(symbols) {
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
  static_cast<void>(typeDefs);
}

void Semantic::analyseVarDecls(const std::vector<VarDecl> &varDecls) {
  static_cast<void>(varDecls);
}

void Semantic::analyseFunctions(
    const std::vector<std::unique_ptr<Function>> &functions) {
  static_cast<void>(functions);
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
  static_cast<void>(statement);
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
  if (condType->getKind() != TypeKind::Boolean)
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
  if (condType->getKind() != TypeKind::Boolean)
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

} // namespace descartes
