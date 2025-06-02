case IROp::Add:
  MATCH_OP(Int64, Scalar, Register, Register, Register,
           [](const Operation &op, auto &out) {
             uint32_t rd = op.operands[0].where & 0x1F;
             uint32_t rn = op.operands[1].where & 0x1F;
             uint32_t rm = op.operands[2].where & 0x1F;
             uint32_t instr = 0b10001011000 << 21 | rm << 16 | rn << 5 |
                              rd; // ADD Xd, Xn, Xm
             emit32(instr, out);
           });
  break;

case IROp::Sub:
  MATCH_OP(Int64, Scalar, Register, Register, Register,
           [](const Operation &op, auto &out) {
             uint32_t rd = op.operands[0].where & 0x1F;
             uint32_t rn = op.operands[1].where & 0x1F;
             uint32_t rm = op.operands[2].where & 0x1F;
             uint32_t instr = 0b11001011000 << 21 | rm << 16 | rn << 5 |
                              rd; // SUB Xd, Xn, Xm
             emit32(instr, out);
           });
  break;

case IROp::Mul:
  MATCH_OP(Int64, Scalar, Register, Register, Register,
           [](const Operation &op, auto &out) {
             uint32_t rd = op.operands[0].where & 0x1F;
             uint32_t rn = op.operands[1].where & 0x1F;
             uint32_t rm = op.operands[2].where & 0x1F;
             uint32_t instr =
                 0x9B000000ull | rm << 16 | rn << 5 | rd; // MUL Xd, Xn, Xm
             emit32(instr, out);
           });
  break;

case IROp::And:
  MATCH_OP(Int64, Scalar, Register, Register, Register,
           [](const Operation &op, auto &out) {
             uint32_t rd = op.operands[0].where & 0x1F;
             uint32_t rn = op.operands[1].where & 0x1F;
             uint32_t rm = op.operands[2].where & 0x1F;
             uint32_t instr = 0b10001010000 << 21 | rm << 16 | rn << 5 |
                              rd; // AND Xd, Xn, Xm
             emit32(instr, out);
           });
  break;

case IROp::Or:
  MATCH_OP(Int64, Scalar, Register, Register, Register,
           [](const Operation &op, auto &out) {
             uint32_t rd = op.operands[0].where & 0x1F;
             uint32_t rn = op.operands[1].where & 0x1F;
             uint32_t rm = op.operands[2].where & 0x1F;
             uint32_t instr = 0b10101010000 << 21 | rm << 16 | rn << 5 |
                              rd; // ORR Xd, Xn, Xm
             emit32(instr, out);
           });
  break;

case IROp::Xor:
  MATCH_OP(Int64, Scalar, Register, Register, Register,
           [](const Operation &op, auto &out) {
             uint32_t rd = op.operands[0].where & 0x1F;
             uint32_t rn = op.operands[1].where & 0x1F;
             uint32_t rm = op.operands[2].where & 0x1F;
             uint32_t instr = 0b11001010000 << 21 | rm << 16 | rn << 5 |
                              rd; // EOR Xd, Xn, Xm
             emit32(instr, out);
           });
  break;

case IROp::Neg:
  MATCH_OP(Int64, Scalar, Register, Register,
           [](const Operation &op, auto &out) {
             // NEG Xd, Xm == SUB Xd, XZR, Xm
             uint32_t rd = op.operands[0].where & 0x1F;
             uint32_t rn = 31; // XZR
             uint32_t rm = op.operands[1].where & 0x1F;
             uint32_t instr = 0b11001011000 << 21 | rm << 16 | rn << 5 | rd;
             emit32(instr, out);
           });
  break;
