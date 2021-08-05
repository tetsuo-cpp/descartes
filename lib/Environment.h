#pragma once

#include <Interfaces.h>
#include <SymbolTable.h>

#include <cassert>

namespace descartes {

// TODO: Rename to `FunctionEntry`.
struct FunctionType {
  FunctionType(const Function *function, const Type *returnType,
               std::vector<const Type *> &&argTypes);
  const Function *function;
  const Type *returnType;
  const std::vector<const Type *> argTypes;
};

class Environment {
public:
  explicit Environment(SymbolTable &symbols);
  virtual ~Environment() = default;

  // TODO: Use a RAII type for this when we begin using exceptions.
  void enterScope();
  void exitScope();
  bool setVarType(Symbol name, const Type *type);
  bool setFunctionType(Symbol name, FunctionType &&function);
  bool setResolvedType(Symbol name, const Type *type);
  const Type *getVarType(Symbol name) const;
  const FunctionType *getFunctionType(Symbol name) const;
  const Type *getResolvedType(Symbol name) const;

private:
  struct Scope {
    std::unordered_map<Symbol, const Type *, SymbolHash> varTypes;
    std::unordered_map<Symbol, const FunctionType, SymbolHash> functionTypes;
    std::unordered_map<Symbol, const Type *, SymbolHash> resolvedTypes;
  };
  std::vector<Scope> scopes;
  std::vector<TypePtr> primitiveTypes;
};

} // namespace descartes
