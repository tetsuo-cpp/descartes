#include "Translate.h"

namespace descartes {

Translate::Translate(const SymbolTable &symbols) : symbols(symbols) {
  static_cast<void>(this->symbols);
}

void Translate::translate(Block &program) { static_cast<void>(program); }

} // namespace descartes
