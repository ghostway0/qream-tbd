#include "qream/ir.h"

using enum IROp;
using enum ScalarDType;
using enum VectorShape;
using enum Space;

#define MATCH_OP2(DTYPE_, SHAPE_, S0_, S1_, LAMBDA_)                      \
  if (op.dtype == DTYPE_ && op.shape == SHAPE_ && op.num_operands >= 2 && \
      op.operands[0].space == S0_ && op.operands[1].space == S1_) {       \
    LAMBDA_(op, out);                                                     \
    break;                                                                \
  }

#define MATCH_OP3(DTYPE_, SHAPE_, S0_, S1_, S2_, LAMBDA_)                 \
  if (op.dtype == DTYPE_ && op.shape == SHAPE_ && op.num_operands >= 3 && \
      op.operands[0].space == S0_ && op.operands[1].space == S1_ &&       \
      op.operands[2].space == S2_) {                                      \
    LAMBDA_(op, out);                                                     \
    break;                                                                \
  }

#define GET_MATCH_MACRO(_1, _2, _3, _4, _5, _6, NAME, ...) NAME
#define MATCH_OP(...) \
  GET_MATCH_MACRO(__VA_ARGS__, MATCH_OP3, MATCH_OP2, MATCH_OP0)(__VA_ARGS__)
