#include "qream/ir.h"

#include <absl/base/log_severity.h>
#include <absl/log/log.h>
#include <absl/log/globals.h>
#include <absl/log/initialize.h>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
  absl::InitializeLog();
  absl::SetStderrThreshold(absl::LogSeverity::kInfo);

  std::vector<Operation> program = {
    {
      .addr = 0x1000,
      .irop = IROp::Add,
      .shape = VectorShape::Scalar,
      .dtype = ScalarDType::Int32,
      .operands = {{
        {AccessType::R, Space::Register, 1},
        {AccessType::R, Space::Register, 2},
        {AccessType::W, Space::Register, 3}
      }},
      .num_operands = 3,
      .predicate = std::nullopt,
    },
    {
      .addr = 0x1004,
      .irop = IROp::Ldr,
      .shape = VectorShape::Scalar,
      .dtype = ScalarDType::Int64,
      .operands = {{
        {AccessType::W, Space::Register, 4},
        {AccessType::R, Space::Memory, 0x2000},
      }},
      .num_operands = 2,
      .predicate = std::nullopt,
    },
    {
      .addr = 0x1008,
      .irop = IROp::Add,
      .shape = VectorShape::V4,
      .dtype = ScalarDType::Float32,
      .operands = {{
        {AccessType::R, Space::Register, 5},
        {AccessType::R, Space::Register, 6},
        {AccessType::W, Space::Register, 7}
      }},
      .num_operands = 3,
      .predicate = Access{AccessType::R, Space::Special, 0},  // IP as predicate
    }
  };

  for (const auto& op : program) {
    std::cout << op << "\n";
  }

  return 0;
}
