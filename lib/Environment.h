#pragma once

#include <Interfaces.h>
#include <Ir.h>
#include <SymbolTable.h>

#include <cassert>

namespace descartes {

// TODO: Consider making an `Environment<T>` type.
// We shouldn't need to know about IR data here.
struct VarEntry {
  VarEntry(const Type *varType, ir::Access access)
      : varType(varType), access(access) {}
  const Type *varType;
  ir::Access access;
};

struct FunctionEntry {
  FunctionEntry(const Function *function, const Type *returnType,
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
  bool setVarType(Symbol name, VarEntry var);
  bool setFunctionType(Symbol name, FunctionEntry &&function);
  bool setResolvedType(Symbol name, const Type *type);
  const VarEntry *getVarType(Symbol name) const;
  const FunctionEntry *getFunctionType(Symbol name) const;
  const Type *getResolvedType(Symbol name) const;

private:
  struct Scope {
    std::unordered_map<Symbol, const VarEntry, SymbolHash> varEntries;
    std::unordered_map<Symbol, const FunctionEntry, SymbolHash> functionEntries;
    std::unordered_map<Symbol, const Type *, SymbolHash> resolvedTypes;
  };
  std::vector<Scope> scopes;
  std::vector<TypePtr> primitiveTypes;
};

} // namespace descartes
