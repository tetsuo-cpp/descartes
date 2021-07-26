#include <Semantic.h>

#include <cassert>

namespace descartes {

// TODO: Remove `const_cast`. Maybe construct the env at the top level and pass
// it in as an arg?
Semantic::Semantic(const SymbolTable &symbols)
    : symbols(symbols), env(const_cast<SymbolTable &>(symbols)) {}

void Semantic::analyse(Block &program) {
  env.enterScope();
  analyseBlock(program);
  env.exitScope();
}

void Semantic::analyseBlock(Block &program) {
  analyseConstDefs(program.constDefs);
  analyseTypeDefs(program.typeDefs);
  analyseVarDecls(program.varDecls);
  analyseFunctions(program.functions);
  analyseBlockStatements(*program.statements);
}

void Semantic::analyseConstDefs(const std::vector<ConstDef> &constDefs) {
  for (const auto &cd : constDefs) {
    const Type *exprType = analyseExpr(*cd.constExpr);
    if (!env.setVarType(cd.identifier, exprType))
      throw SemanticError("Const already defined");
  }
}

void Semantic::analyseTypeDefs(const std::vector<TypeDef> &typeDefs) {
  for (const auto &td : typeDefs) {
    const Type *resolvedType = td.type.get();
    if (resolvedType->getKind() == TypeKind::Alias) {
      const Symbol aliasIdentifier =
          static_cast<const Alias *>(resolvedType)->typeIdentifier;
      resolvedType = env.getResolvedType(aliasIdentifier);
    }
    if (!resolvedType)
      throw SemanticError("Could not resolve type");
    if (!env.setResolvedType(td.identifier, resolvedType))
      throw SemanticError("Type already defined");
  }
}

void Semantic::analyseVarDecls(const std::vector<VarDecl> &varDecls) {
  for (const auto &vd : varDecls) {
    const Type *varType = env.getResolvedType(vd.type);
    if (!varType)
      throw SemanticError("Could not find type of variable");
    if (!env.setVarType(vd.identifier, varType))
      throw SemanticError("Variable already defined");
  }
}

void Semantic::analyseFunctions(
    const std::vector<std::unique_ptr<Function>> &functions) {
  // First capture the function signatures.
  for (const auto &f : functions) {
    // Resolve the types associated with this function.
    const Type *returnType = nullptr;
    if (f->returnType) {
      returnType = env.getResolvedType(*f->returnType);
      if (!returnType)
        throw SemanticError("Could not resolve return type");
    }
    std::vector<const Type *> argTypes;
    for (const auto &arg : f->args) {
      const Type *argType = env.getResolvedType(arg.type);
      if (!argType)
        throw SemanticError("Could not resolve type of argument");
      argTypes.push_back(argType);
    }
    // Set the function type so outer callers can use it.
    FunctionType functionType(f.get(), returnType, std::move(argTypes));
    env.setFunctionType(f->name, std::move(functionType));
  }
  // Now analyse each function block.
  for (const auto &f : functions) {
    env.enterScope();
    const FunctionType *functionType = env.getFunctionType(f->name);
    if (functionType->returnType)
      // Is this the right spot here? In Pascal, functions have a variable with
      // the same name as the function itself that is used to capture the return
      // value.
      if (!env.setVarType(f->name, functionType->returnType))
        throw SemanticError("Return value already defined");
    // Register each param as a variable.
    for (size_t i = 0; i < f->args.size(); ++i)
      if (!env.setVarType(f->args.at(i).identifier,
                          functionType->argTypes.at(i)))
        throw SemanticError("Argument already defined");
    // Now semantically analyse the associated nested functions and blocks.
    analyseBlock(f->block);
    env.exitScope();
  }
}

void Semantic::analyseBlockStatements(Statement &statement) {
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
  if (condType->getKind() != TypeKind::Boolean)
    throw SemanticError("If condition must be boolean");
  analyseStatement(*ifStatement->thenStatement);
  if (ifStatement->elseStatement)
    analyseStatement(*ifStatement->elseStatement);
}

void Semantic::analyseCase(Statement &statement) {
  // TODO: Implement case statements.
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
  const auto *stringType = env.getResolvedType(*symbols.lookup("string"));
  assert(stringType && stringType->getKind() == TypeKind::String);
  return stringType;
}

const Type *Semantic::analyseNumberLiteral(Expr &expr) {
  auto *numberLiteral = exprCast<NumberLiteral *>(expr);
  assert(numberLiteral);
  const auto *numberType = env.getResolvedType(*symbols.lookup("integer"));
  assert(numberType && numberType->getKind() == TypeKind::Integer);
  return numberType;
}

const Type *Semantic::analyseVarRef(Expr &expr) {
  auto *varRef = exprCast<VarRef *>(expr);
  assert(varRef);
  const auto *varType = env.getVarType(varRef->identifier);
  if (!varType)
    throw SemanticError("Referencing unknown variable");
  return varType;
}

const Type *Semantic::analyseBinaryOp(Expr &expr) {
  auto *binaryOp = exprCast<BinaryOp *>(expr);
  assert(binaryOp);
  const Type *lhs = analyseExpr(*binaryOp->lhs),
             *rhs = analyseExpr(*binaryOp->rhs);
  const Type *integerType = env.getResolvedType(*symbols.lookup("integer"));
  const Type *boolType = env.getResolvedType(*symbols.lookup("boolean"));
  switch (binaryOp->kind) {
  case BinaryOpKind::Add:
  case BinaryOpKind::Subtract:
  case BinaryOpKind::Multiply:
  case BinaryOpKind::Divide: {
    // Must be integers.
    if (lhs->getKind() != TypeKind::Integer ||
        rhs->getKind() != TypeKind::Integer)
      throw SemanticError("Expected integer in binary op");
    return integerType;
  }
  case BinaryOpKind::LessThan:
  case BinaryOpKind::GreaterThan:
  case BinaryOpKind::LessThanEqual:
  case BinaryOpKind::GreaterThanEqual: {
    // Must be integers.
    if (lhs->getKind() != TypeKind::Integer ||
        rhs->getKind() != TypeKind::Integer)
      throw SemanticError("Expected integer in binary op");
    return boolType;
  }
  case BinaryOpKind::Equal:
  case BinaryOpKind::NotEqual:
    // Can be integers, strings or booleans.
    const auto lhsKind = lhs->getKind(), rhsKind = rhs->getKind();
    if (lhsKind != rhsKind)
      throw SemanticError("Mismatching types in equality");
    if (lhsKind != TypeKind::Integer && lhsKind != TypeKind::String &&
        lhsKind != TypeKind::Boolean)
      throw SemanticError("Expected integer, string or boolean in equality");
    return boolType;
  }
  return nullptr;
}

const Type *Semantic::analyseCall(Expr &expr) {
  auto *call = exprCast<Call *>(expr);
  assert(call);
  // Get function.
  const FunctionType *function = env.getFunctionType(call->functionName);
  if (!function)
    throw SemanticError("Unknown function");
  if (function->argTypes.size() != call->args.size())
    throw SemanticError("Wrong number of args");
  for (size_t i = 0; i < call->args.size(); ++i) {
    const Type *providedType = analyseExpr(*call->args[i]);
    const Type *fArg = function->argTypes[i];
    if (!isCompatibleType(fArg, providedType))
      throw SemanticError("Gave function wrong type");
  }
  // Nullptr is fine.
  return function->returnType;
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
      const Type *memberType = env.getResolvedType(member.second);
      if (!memberType)
        // Maybe do this eagerly instead of waiting for a member access?
        throw SemanticError("Member of unknown type");
      return memberType;
    }
  }
  throw SemanticError("Can't find the right member on the record type");
}

bool Semantic::isCompatibleType(const Type *lhs, const Type *rhs) const {
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
