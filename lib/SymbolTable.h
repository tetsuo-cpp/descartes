#pragma once

#include <Interfaces.h>

#include <optional>
#include <string>
#include <unordered_map>

namespace descartes {

class SymbolTable {
public:
  SymbolTable();
  virtual ~SymbolTable() = default;
  Symbol make(const std::string &name);
  std::optional<Symbol> lookup(const std::string &name) const;

private:
  int currentId;
  std::unordered_map<std::string, Symbol> symbolMap;
};

} // namespace descartes
