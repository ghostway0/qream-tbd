case IROp::Add:
  MATCH_OP(Int64, Scalar, Register, Register, Register,
           [](const Operation &op, auto &out) {
             emit_3reg(0b10001011000, op.operands[0].where & 0x1F,
                       op.operands[1].where & 0x1F,
                       op.operands[2].where & 0x1F, out);
           });
  break;

case IROp::Sub:
  MATCH_OP(Int64, Scalar, Register, Register, Register,
           [](const Operation &op, auto &out) {
             emit_3reg(0b11001011000, op.operands[0].where & 0x1F,
                       op.operands[1].where & 0x1F,
                       op.operands[2].where & 0x1F, out);
           });
  break;

case IROp::Mul:
  MATCH_OP(Int64, Scalar, Register, Register, Register,
           [](const Operation &op, auto &out) {
             uint32_t rd = op.operands[0].where & 0x1F;
             uint32_t rn = op.operands[1].where & 0x1F;
             uint32_t rm = op.operands[2].where & 0x1F;
             emit_3reg(0b10011011000, rd, rn, rm, out);
           });
  break;

case IROp::And:
  MATCH_OP(Int64, Scalar, Register, Register, Register,
           [](const Operation &op, auto &out) {
             emit_3reg(0b10001010000, op.operands[0].where & 0x1F,
                       op.operands[1].where & 0x1F,
                       op.operands[2].where & 0x1F, out);
           });
  break;

case IROp::Or:
  MATCH_OP(Int64, Scalar, Register, Register, Register,
           [](const Operation &op, auto &out) {
             emit_3reg(0b10101010000, op.operands[0].where & 0x1F,
                       op.operands[1].where & 0x1F,
                       op.operands[2].where & 0x1F, out);
           });
  break;

case IROp::Xor:
  MATCH_OP(Int64, Scalar, Register, Register, Register,
           [](const Operation &op, auto &out) {
             emit_3reg(0b11001010000, op.operands[0].where & 0x1F,
                       op.operands[1].where & 0x1F,
                       op.operands[2].where & 0x1F, out);
           });
  break;

case IROp::Neg:
  MATCH_OP(Int64, Scalar, Register, Register,
           [](const Operation &op, auto &out) {
             emit_3reg(0b11001011000, op.operands[0].where & 0x1F,
                       31, // XZR
                       op.operands[1].where & 0x1F, out);
           });
  break;
