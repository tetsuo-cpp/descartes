#pragma once

#include <Interfaces.h>

namespace descartes {

class Semantic {
public:
  virtual ~Semantic() = default;
  void analyse(Block &program);
};

} // namespace descartes
