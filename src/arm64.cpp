#include <vector>
#include <absl/log/log.h>

#include "qream/ir.h"
#include "qream/match.h"

inline void emit_3reg(uint32_t opcode21, uint32_t rd, uint32_t rn, uint32_t rm, auto &out) {
  // [31:21] opcode [20:16] Rm [15:10] unused [9:5] Rn [4:0] Rd
  uint32_t instr = (opcode21 << 21) | (rm << 16) | (rn << 5) | rd;
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
