#include "Environment.h"

namespace descartes {

FunctionType::FunctionType(const Function *function, const Type *returnType,
                           std::vector<const Type *> &&argTypes)
    : function(function), returnType(returnType),
      argTypes(std::move(argTypes)) {}

void Environment::enterScope() { scopes.emplace_back(); }

void Environment::exitScope() {
  assert(!scopes.empty());
  scopes.pop_back();
}

bool Environment::setVarType(Symbol name, const Type *type) {
  assert(!scopes.empty());
  auto &currentScope = scopes.back();
  auto iter = currentScope.varTypes.find(name);
  if (iter != currentScope.varTypes.end())
    return false;
  currentScope.varTypes.emplace(name, type);
  return true;
}

bool Environment::setFunctionType(Symbol name, FunctionType &&function) {
  assert(!scopes.empty());
  auto &currentScope = scopes.back();
  auto iter = currentScope.functionTypes.find(name);
  if (iter != currentScope.functionTypes.end())
    return false;
  currentScope.functionTypes.emplace(name, std::move(function));
  return true;
}

bool Environment::setResolvedType(Symbol name, const Type *type) {
  assert(!scopes.empty());
  auto &currentScope = scopes.back();
  auto iter = currentScope.resolvedTypes.find(name);
  if (iter != currentScope.resolvedTypes.end())
    return false;
  currentScope.resolvedTypes.emplace(name, type);
  return true;
}

const Type *Environment::getVarType(Symbol name) const {
  assert(!scopes.empty());
  // Iterate backwards.
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    const auto &scope = *it;
    const auto varIter = scope.varTypes.find(name);
    if (varIter != scope.varTypes.end())
      return varIter->second;
  }
  return nullptr;
}

const FunctionType *Environment::getFunctionType(Symbol name) const {
  assert(!scopes.empty());
  // Iterate backwards.
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    const auto &scope = *it;
    const auto funcIter = scope.functionTypes.find(name);
    if (funcIter != scope.functionTypes.end())
      return &funcIter->second;
  }
  return nullptr;
}

const Type *Environment::getResolvedType(Symbol name) const {
  assert(!scopes.empty());
  // Iterate backwards.
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    const auto &scope = *it;
    const auto typeIter = scope.resolvedTypes.find(name);
    if (typeIter != scope.resolvedTypes.end())
      return typeIter->second;
  }
  return nullptr;
}

} // namespace descartes
