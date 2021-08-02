#pragma once

#include "Ir.h"

namespace descartes {

class Translate {
public:
  explicit Translate(const SymbolTable &symbols);
  virtual ~Translate() = default;
  void translate(Block &program);

private:
  const SymbolTable &symbols;
};

} // namespace descartes
