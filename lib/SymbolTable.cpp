#include "SymbolTable.h"

#include <cassert>

namespace descartes {

SymbolTable::SymbolTable() : currentId(0) {}

Symbol SymbolTable::make(const std::string &name) {
  const auto iter = symbolMap.find(name);
  if (iter != symbolMap.end())
    return iter->second;
  auto entry = std::make_pair(name, Symbol(currentId++));
  entry.second.value = &entry.first;
  auto result = symbolMap.insert(std::move(entry));
  assert(result.second);
  return result.first->second;
}

std::optional<Symbol> SymbolTable::lookup(const std::string &name) const {
  const auto iter = symbolMap.find(name);
  if (iter != symbolMap.end())
    return iter->second;
  return {};
}

} // namespace descartes
