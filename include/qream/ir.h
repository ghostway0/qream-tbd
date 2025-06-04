#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <ostream>
#include <variant>

using Address = uint64_t;

enum class ScalarDType : uint8_t {
  Int8,
  Int16,
  Int32,
  Int64,
  Float16,
  Float32,
  Float64,
};

enum class Space {
  Registers,
  Memory,
  Immediate,
  Special,
};

enum class VectorShape : uint8_t {
  Scalar = 1,
  V2 = 2,
  V4 = 4,
  V8 = 8,
  V16 = 16,
};

using SizeClass = uint8_t;

struct Register {
  uint8_t enc;
  SizeClass size_class;
};

struct MemoryAddressing {
  std::optional<Register> base_reg;
  std::optional<Register> index;
  uint64_t offset;
};

enum class Standard {
  PC,
  SP,
};

using Imm64 = uint64_t;

using Access = std::variant<MemoryAddressing, Register, Imm64, Standard>;

// bool operator==(const Access &a, const Access &b);

enum class IROp {
  Xchg,
  Str,
  Ldr,
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Neg,
  And,
  Or,
  Xor,
  Not,
  Shl,
  Shr,
  Rol,
  Ror,
  Jump,
  JumpIf,
  Call,
  Ret,
  Trap,
  Halt,
  FAdd,
  FSub,
  FMul,
  FDiv,
  SignExtend,
  ZeroExtend,
  Truncate,
  Fence,
  AtomicAdd,
  AtomicCmpXchg,
  VShuffle,   // permute elements
  VBlend,     // masked merge
  VExtract,   // extract lane
  VInsert,    // insert lane
  VReduceAdd, // horizontal reduction
};

struct Operation {
  Address addr;
  IROp irop;
  VectorShape shape = VectorShape::Scalar;
  ScalarDType dtype;
  std::array<Access, 3> operands;
  size_t num_operands;
  std::optional<Access> predicate;

  std::string toString() const;
};

std::ostream &operator<<(std::ostream &os, const ScalarDType &dtype);
std::ostream &operator<<(std::ostream &os, const Access &access);
std::ostream &operator<<(std::ostream &os, const IROp &op);
std::ostream &operator<<(std::ostream &os, const Operation &op);
