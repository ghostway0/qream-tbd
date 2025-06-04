#include "qream/ir.h"
#include "qream/arm64.h"
#include <sys/mman.h> // mmap, munmap, PROT_*, MAP_*
#include <unistd.h>   // getpagesize, if needed

#include <absl/base/log_severity.h>
#include <iomanip>
#include <absl/log/log.h>
#include <absl/log/globals.h>
#include <absl/log/initialize.h>
#include <iostream>
#include <vector>

void print_hex(const std::vector<uint8_t> &code) {
  for (uint8_t b : code) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(b) << ' ';
  }
  std::cout << std::dec << '\n';
}

int main() {
  std::vector<Operation> ops = {
      Operation{0,
                IROp::Add,
                VectorShape::Scalar,
                ScalarDType::Int64,
                {Register{7, 3}, Register{1, 3}, Register{2, 3}},
                3},
      Operation{1,
                IROp::Sub,
                VectorShape::Scalar,
                ScalarDType::Int64,
                {Register{3, 3}, Register{1, 3}, Register{2, 3}},
                3},
      Operation{2,
                IROp::Mul,
                VectorShape::Scalar,
                ScalarDType::Int64,
                {Register{4, 8}, Register{1, 3}, Register{2, 3}},
                3},
      Operation{3,
                IROp::Xor,
                VectorShape::Scalar,
                ScalarDType::Int64,
                {Register{5, 3}, Register{1, 3}, Register{2, 3}},
                3},
      Operation{4,
                IROp::Neg,
                VectorShape::Scalar,
                ScalarDType::Int64,
                {Register{6, SizeClass(3)}, Register{1, SizeClass(3)}},
                2},
  };

  for (const Operation &op : ops) {
    std::cerr << op << "\n";
  }

  std::vector<uint8_t> code = transpile_to_arm64(ops);
  std::cout << "Machine code:\n";
  print_hex(code);

  uint32_t ret_instr = 0xD65F03C0; // RET
  code.push_back(ret_instr & 0xFF);
  code.push_back((ret_instr >> 8) & 0xFF);
  code.push_back((ret_instr >> 16) & 0xFF);
  code.push_back((ret_instr >> 24) & 0xFF);

  void *mem = mmap(nullptr, code.size(), PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(mem != MAP_FAILED);
  std::memcpy(mem, code.data(), code.size());

  mprotect(mem, code.size(), PROT_READ | PROT_EXEC);

  uint64_t x1 = 10, x2 = 3;
  uint64_t x0, x3, x4, x5, x6;

  asm volatile(
      "mov x1, %0\n"
      "mov x2, %1\n"
      :
      : "r"(x1), "r"(x2)
      : "x1", "x2");

  using Fn = void (*)();
  ((Fn)mem)();

  asm volatile(
      "mov %0, x7\n"
      "mov %1, x3\n"
      "mov %2, x4\n"
      "mov %3, x5\n"
      "mov %4, x6\n"
      : "=r"(x0), "=r"(x3), "=r"(x4), "=r"(x5), "=r"(x6));

  std::cout << "x7 (add): " << x0 << "\n"; // 13
  std::cout << "x3 (sub): " << x3 << "\n"; // 7
  std::cout << "x4 (mul): " << x4 << "\n"; // 30
  std::cout << "x5 (xor): " << x5 << "\n"; // 9 ^ 3 = 9
  std::cout << "x6 (neg): " << x6 << "\n"; // -10

  assert(x0 == 13);
  assert(x3 == 7);
  assert(x4 == 30);
  assert(x5 == (x1 ^ x2));
  assert(x6 == (uint64_t)-10);

  std::cout << "All tests passed.\n";
  munmap(mem, code.size());
}
