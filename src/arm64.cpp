#include <vector>
#include <absl/log/log.h>

#include "qream/ir.h"
#include "qream/match.h"

inline void emit32(uint32_t instr, auto &out) {
  out++ = instr & 0xFF;
  out++ = (instr >> 8) & 0xFF;
  out++ = (instr >> 16) & 0xFF;
  out++ = (instr >> 24) & 0xFF;
}

std::vector<uint8_t> transpile_to_arm64(const std::vector<Operation> &ops) {
  std::vector<uint8_t> code;
  auto out = std::back_inserter(code);

  for (const auto &op : ops) {
    switch (op.irop) {
#include "arm64.inc"
      default:
        LOG(WARNING) << "Unsupported IROp\n";
        break;
    }
  }

  return code;
}
