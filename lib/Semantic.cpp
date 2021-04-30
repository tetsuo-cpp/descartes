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
  switch (expr.getKind()) {
  case ExprKind::StringLiteral:
    return analyseStringLiteral(expr);
  case ExprKind::NumberLiteral:
    return analyseNumberLiteral(expr);
  case ExprKind::VarRef:
    return analyseVarRef(expr);
  case ExprKind::BinaryOp:
    return analyseBinaryOp(expr);
  case ExprKind::Call:
    return analyseCall(expr);
  case ExprKind::MemberRef:
    return analyseMemberRef(expr);
  }
  return nullptr;
}

const Type *Semantic::analyseStringLiteral(Expr &expr) {
  auto *stringLiteral = exprCast<StringLiteral *>(expr);
  assert(stringLiteral);
  auto iter = typeDefs->find(*symbols.lookup("string"));
  assert(iter != typeDefs->end() &&
         iter->second->getKind() == TypeKind::String);
  return iter->second.get();
}

const Type *Semantic::analyseNumberLiteral(Expr &expr) {
  auto *numberLiteral = exprCast<NumberLiteral *>(expr);
  assert(numberLiteral);
  auto iter = typeDefs->find(*symbols.lookup("integer"));
  assert(iter != typeDefs->end() &&
         iter->second->getKind() == TypeKind::Integer);
  return iter->second.get();
}

const Type *Semantic::analyseVarRef(Expr &expr) {
  auto *varRef = exprCast<VarRef *>(expr);
  assert(varRef);
  auto iter = varDecls->find(varRef->identifier);
  if (iter == varDecls->end())
    throw SemanticError("Referencing unknown variable");
  Symbol varTypeIdentifier = iter->second;
  auto typeIter = typeDefs->find(varTypeIdentifier);
  if (typeIter == typeDefs->end())
    throw SemanticError("Variable of unknown type");
  return typeIter->second.get();
}

const Type *Semantic::analyseBinaryOp(Expr &expr) {
  auto *binaryOp = exprCast<BinaryOp *>(expr);
  assert(binaryOp);
  const Type *lhs = analyseExpr(*binaryOp->lhs),
             *rhs = analyseExpr(*binaryOp->rhs);
  auto iter = typeDefs->find(*symbols.lookup("boolean"));
  assert(iter != typeDefs->end());
  switch (binaryOp->kind) {
  case BinaryOpKind::Add:
  case BinaryOpKind::Subtract:
  case BinaryOpKind::Multiply:
  case BinaryOpKind::Divide:
  case BinaryOpKind::LessThan:
  case BinaryOpKind::GreaterThan:
  case BinaryOpKind::LessThanEqual:
  case BinaryOpKind::GreaterThanEqual: {
    // Must be integers.
    if (resolveType(lhs)->getKind() != TypeKind::Integer ||
        resolveType(rhs)->getKind() != TypeKind::Integer)
      throw SemanticError("Expected integer in binary op");
    return iter->second.get();
  }
  case BinaryOpKind::Equal:
  case BinaryOpKind::NotEqual:
    // Can be integers, strings or booleans.
    const auto lhsKind = resolveType(lhs)->getKind(),
               rhsKind = resolveType(rhs)->getKind();
    if (lhsKind != rhsKind)
      throw SemanticError("Mismatching types in equality");
    if (lhsKind != TypeKind::Integer && lhsKind != TypeKind::String &&
        lhsKind != TypeKind::Boolean)
      throw SemanticError("Expected integer, string or boolean in equality");
    return iter->second.get();
  }
  return nullptr;
}

const Type *Semantic::analyseCall(Expr &expr) {
  auto *call = exprCast<Call *>(expr);
  assert(call);
  // Get function.
  Function *function = nullptr;
  for (const auto &f : *functions) {
    if (call->functionName == f->name) {
      function = f.get();
      break;
    }
  }
  if (!function)
    throw SemanticError("Unknown function");
  if (function->args.size() != call->args.size())
    throw SemanticError("Wrong number of args");
  for (size_t i = 0; i < call->args.size(); ++i) {
    const Type *providedType = analyseExpr(*call->args[i]);
    const FunctionArg fArg = function->args[i];
    auto iter = typeDefs->find(fArg.type);
    if (iter == typeDefs->end())
      throw SemanticError("Function takes unknown type");
    if (!isCompatibleType(resolveType(iter->second.get()),
                          resolveType(providedType)))
      throw SemanticError("Gave function wrong type");
  }
  if (function->returnType) {
    auto iter = typeDefs->find(*function->returnType);
    if (iter == typeDefs->end())
      throw SemanticError("Unknown return type");
    return resolveType(iter->second.get());
  }
  return nullptr;
}

const Type *Semantic::analyseMemberRef(Expr &expr) {
  auto *memberRef = exprCast<MemberRef *>(expr);
  assert(memberRef);
  const Type *exprType = analyseExpr(*memberRef->expr);
  if (exprType->getKind() != TypeKind::Record)
    throw SemanticError("Member ref access on non-record type");
  const Record *recordType = static_cast<const Record *>(exprType);
  for (const auto &member : recordType->fields) {
    if (member.first == memberRef->identifier) {
      // Found the member.
      auto iter = typeDefs->find(member.second);
      if (iter == typeDefs->end()) {
        // Maybe do this eagerly instead of waiting for a member access?
        throw SemanticError("Member of unknown type");
      }
      return resolveType(iter->second.get());
    }
  }
  throw SemanticError("Can't find the right member on the record type");
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
  case TypeKind::String:
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
