#pragma once

#include <absl/status/status.h>

#include "qream/ir.h"

using enum IROp;
using enum ScalarDType;
using enum VectorShape;

template <typename... OperandTypes>
struct extract_operands {
  template <typename Handler, typename OutputIt>
  static bool try_match(const Operation &op, Handler &&handler,
                        OutputIt &out) {
    static_assert(sizeof...(OperandTypes) <= 3, "Max 3 operands supported");

    if (op.num_operands < sizeof...(OperandTypes)) {
      return false;
    }

    return try_extract(op, std::forward<Handler>(handler), out,
                       std::make_index_sequence<sizeof...(OperandTypes)>{});
  }

 private:
  template <typename Handler, typename OutputIt, size_t... Is>
  static bool try_extract(const Operation &op, Handler &&handler,
                          OutputIt &out, std::index_sequence<Is...>) {
    auto ptrs =
        std::make_tuple(std::get_if<OperandTypes>(&op.operands[Is])...);
    if ((... && (std::get<Is>(ptrs) != nullptr))) {
      handler(op, *std::get<Is>(ptrs)..., out);
      return true;
    }
    return false;
  }
};

template <ScalarDType dtype, VectorShape shape, typename... OperandTypes,
          typename Handler, typename OutputIt>
absl::Status match_op(const Operation &op, Handler &&handler,
                      OutputIt &out) {
  if (op.dtype != dtype || op.shape != shape) {
    return absl::InvalidArgumentError("Op signature doesn't match");
  }

  if (!extract_operands<OperandTypes...>::try_match(
          op, std::forward<Handler>(handler), out)) {
    return absl::InvalidArgumentError("Op signature doesn't match");
  }

  return absl::OkStatus();
}

#define MATCH_OP(DTYPE, SHAPE, ...) match_op<DTYPE, SHAPE, __VA_ARGS__>
