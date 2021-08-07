#include <Semantic.h>

#include <cassert>

namespace descartes {

// TODO: Remove `const_cast`. Maybe construct the env at the top level and pass
// it in as an arg?
Semantic::Semantic(const SymbolTable &symbols)
    : symbols(symbols), env(const_cast<SymbolTable &>(symbols)),
      translate(const_cast<SymbolTable &>(symbols)) {}

const std::vector<ir::Fragment> &Semantic::analyse(Block &program) {
  env.enterScope();
  analyseBlock(program);
  env.exitScope();
  return translate.getFrags();
}

void Semantic::analyseBlock(Block &block) {
  analyseConstDefs(block.constDefs);
  analyseTypeDefs(block.typeDefs);
  analyseVarDecls(block.varDecls);
  analyseFunctions(block.functions);
  analyseBlockStatements(*block.statements);
}

void Semantic::analyseConstDefs(const std::vector<ConstDef> &constDefs) {
  for (const auto &cd : constDefs) {
    const auto exprType = analyseExpr(*cd.constExpr);
    if (!env.setVarType(cd.identifier, exprType.second))
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

ir::StatementPtr Semantic::analyseStatement(Statement &statement) {
  switch (statement.getKind()) {
  case StatementKind::Assignment:
    return analyseAssignment(statement);
  case StatementKind::Compound:
    return analyseCompound(statement);
  case StatementKind::If:
    return analyseIf(statement);
  case StatementKind::Case:
    return analyseCase(statement);
  case StatementKind::While:
    return analyseWhile(statement);
  case StatementKind::Call:
    return analyseCallStatement(statement);
  default:
    throw SemanticError("Unsupported statement kind");
  }
}

ir::StatementPtr Semantic::analyseAssignment(Statement &statement) {
  // TODO: Handle const.
  auto *assignment = statementCast<Assignment *>(statement);
  assert(assignment);
  auto lhs = analyseExpr(*assignment->lhs), rhs = analyseExpr(*assignment->rhs);
  if (!isCompatibleType(lhs.second, rhs.second))
    throw SemanticError("Assignment error");
  auto moveVal = translate.makeMove(std::move(lhs.first), std::move(rhs.first));
  return moveVal;
}

ir::StatementPtr Semantic::analyseCompound(Statement &statement) {
  auto *compound = statementCast<Compound *>(statement);
  assert(compound);
  std::vector<ir::StatementPtr> body;
  for (const auto &s : compound->body)
    body.push_back(analyseStatement(*s));
  return translate.makeSequence(std::move(body));
}

ir::StatementPtr Semantic::analyseIf(Statement &statement) {
  auto *ifStatement = statementCast<If *>(statement);
  assert(ifStatement);
  auto condType = analyseExpr(*ifStatement->cond);
  if (condType.second->getKind() != TypeKind::Boolean)
    throw SemanticError("If condition must be boolean");
  // Check whether we're checking a boolean value or return value OR there's a
  // relational check here.
  auto &condVal = condType.first;
  ir::StatementPtr thenVal = analyseStatement(*ifStatement->thenStatement),
                   elseVal;
  if (ifStatement->elseStatement)
    elseVal = analyseStatement(*ifStatement->elseStatement);
  return translate.makeIf(std::move(condVal), std::move(thenVal),
                          std::move(elseVal));
}

ir::StatementPtr Semantic::analyseCase(Statement &statement) {
  // TODO: Implement case statements.
  static_cast<void>(statement);
  throw SemanticError("Case statements not implemented");
}

ir::StatementPtr Semantic::analyseWhile(Statement &statement) {
  auto *whileStatement = statementCast<While *>(statement);
  assert(whileStatement);
  auto condType = analyseExpr(*whileStatement->cond);
  if (condType.second->getKind() != TypeKind::Boolean)
    throw SemanticError("While condition must be a boolean");
  auto bodyVal = analyseStatement(*whileStatement->body);
  auto whileVal =
      translate.makeWhile(std::move(condType.first), std::move(bodyVal));
  return whileVal;
}

ir::StatementPtr Semantic::analyseCallStatement(Statement &statement) {
  auto *callStatement = statementCast<CallStatement *>(statement);
  assert(callStatement);
  auto *call = exprCast<Call *>(*callStatement->call);
  if (!call)
    throw SemanticError("Call statement with a non-call node within");
  auto callVal = analyseExpr(*call);
  return translate.makeCallStatement(std::move(callVal.first));
}

Semantic::ExprResult Semantic::analyseExpr(Expr &expr) {
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
  throw SemanticError("Unknown expr type");
}

Semantic::ExprResult Semantic::analyseStringLiteral(Expr &expr) {
  auto *stringLiteral = exprCast<StringLiteral *>(expr);
  assert(stringLiteral);
  const auto *stringType = env.getResolvedType(*symbols.lookup("string"));
  assert(stringType && stringType->getKind() == TypeKind::String);
  auto nameVal = translate.makeName(*stringLiteral);
  return {std::move(nameVal), stringType};
}

Semantic::ExprResult Semantic::analyseNumberLiteral(Expr &expr) {
  auto *numberLiteral = exprCast<NumberLiteral *>(expr);
  assert(numberLiteral);
  const auto *numberType = env.getResolvedType(*symbols.lookup("integer"));
  assert(numberType && numberType->getKind() == TypeKind::Integer);
  auto constVal = translate.makeConst(*numberLiteral);
  return {std::move(constVal), numberType};
}

Semantic::ExprResult Semantic::analyseVarRef(Expr &expr) {
  auto *varRef = exprCast<VarRef *>(expr);
  assert(varRef);
  const auto *varType = env.getVarType(varRef->identifier);
  if (!varType)
    throw SemanticError("Referencing unknown variable");
  // TODO: Add accesses to env.
  return {nullptr, nullptr};
}

Semantic::ExprResult Semantic::analyseBinaryOp(Expr &expr) {
  auto *binaryOp = exprCast<BinaryOp *>(expr);
  assert(binaryOp);
  auto lhs = analyseExpr(*binaryOp->lhs), rhs = analyseExpr(*binaryOp->rhs);
  const Type *integerType = env.getResolvedType(*symbols.lookup("integer")),
             *boolType = env.getResolvedType(*symbols.lookup("boolean"));
  switch (binaryOp->kind) {
  case BinaryOpKind::Add:
  case BinaryOpKind::Subtract:
  case BinaryOpKind::Multiply:
  case BinaryOpKind::Divide: {
    // Must be integers.
    if (lhs.second->getKind() != TypeKind::Integer ||
        rhs.second->getKind() != TypeKind::Integer)
      throw SemanticError("Expected integer in binary op");
    auto binOpVal = translate.makeArithOp(binaryOp->kind, std::move(lhs.first),
                                          std::move(rhs.first));
    return {std::move(binOpVal), integerType};
  }
  case BinaryOpKind::LessThan:
  case BinaryOpKind::GreaterThan:
  case BinaryOpKind::LessThanEqual:
  case BinaryOpKind::GreaterThanEqual: {
    // Must be integers.
    if (lhs.second->getKind() != TypeKind::Integer ||
        rhs.second->getKind() != TypeKind::Integer)
      throw SemanticError("Expected integer in binary op");
    auto relOpVal = translate.makeCondJump(binaryOp->kind, std::move(lhs.first),
                                           std::move(rhs.first));
    // TODO: Reassess the use of `CondJump`.
    // At the moment, I'm intending to just leave the labels blank when it's not
    // used in a conditional. If that works fine, then consider renaming this to
    // something else since it's not necessarily jumping anywhere.
    return {std::move(relOpVal), boolType};
  }
  case BinaryOpKind::Equal:
  case BinaryOpKind::NotEqual:
    // Can be integers, strings or booleans.
    const auto lhsKind = lhs.second->getKind(), rhsKind = rhs.second->getKind();
    if (lhsKind != rhsKind)
      throw SemanticError("Mismatching types in equality");
    if (lhsKind != TypeKind::Integer && lhsKind != TypeKind::String &&
        lhsKind != TypeKind::Boolean)
      throw SemanticError("Expected integer, string or boolean in equality");
    auto relOpVal = translate.makeCondJump(binaryOp->kind, std::move(lhs.first),
                                           std::move(rhs.first));
    return {std::move(relOpVal), boolType};
  }
  throw SemanticError("Unknown binary op");
}

Semantic::ExprResult Semantic::analyseCall(Expr &expr) {
  auto *call = exprCast<Call *>(expr);
  assert(call);
  // Get function.
  const FunctionType *function = env.getFunctionType(call->functionName);
  if (!function)
    throw SemanticError("Unknown function");
  if (function->argTypes.size() != call->args.size())
    throw SemanticError("Wrong number of args");
  std::vector<ir::ExprPtr> argVals;
  for (size_t i = 0; i < call->args.size(); ++i) {
    auto providedType = analyseExpr(*call->args[i]);
    const Type *fArg = function->argTypes[i];
    if (!isCompatibleType(fArg, providedType.second))
      throw SemanticError("Gave function wrong type");
    argVals.push_back(std::move(providedType.first));
  }
  auto callVal =
      std::make_unique<ir::Call>(call->functionName, std::move(argVals));
  // Nullptr is fine.
  return {std::move(callVal), function->returnType};
}

Semantic::ExprResult Semantic::analyseMemberRef(Expr &expr) {
  auto *memberRef = exprCast<MemberRef *>(expr);
  assert(memberRef);
  const auto exprType = analyseExpr(*memberRef->expr);
  if (exprType.second->getKind() != TypeKind::Record)
    throw SemanticError("Member ref access on non-record type");
  const Record *recordType = static_cast<const Record *>(exprType.second);
  for (const auto &member : recordType->fields) {
    if (member.first == memberRef->identifier) {
      // Found the member.
      const Type *memberType = env.getResolvedType(member.second);
      if (!memberType)
        // Maybe do this eagerly instead of waiting for a member access?
        throw SemanticError("Member of unknown type");
      // TODO: Do this when I figure out accesses for normal variables.
      return {nullptr, memberType};
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
